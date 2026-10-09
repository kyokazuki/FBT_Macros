#include <iostream>
#include <stdlib.h>

#include <TCanvas.h>
#include <TGraph.h>
#include <TMath.h>
#include <TF1.h>
#include <TH2F.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TCanvas *c1 = nullptr;
vector<TH1F*> hDxi(3, nullptr);
vector<TH1F*> hDtime(3, nullptr);
vector<TH2F*> hDxiDtime(3, nullptr);

void plotClusterStart(
	const DataFBT2& inData,
	const Float_t (&totRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete c1;
	c1 = new TCanvas("c1", "c1", 800, 600);

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hDxi[layer];
		hDxi[layer] = new TH1F(
			Form("hDxi%c", LAYER_NAMES[layer]),
			Form("dxi%c;xi%c[1] - xi%c[0]", LAYER_NAMES[layer], LAYER_NAMES[layer], LAYER_NAMES[layer]),
			639, -319.5, 319.5
		);

		delete hDtime[layer];
		hDtime[layer] = new TH1F(
			Form("hDtime%c", LAYER_NAMES[layer]),
			Form("dtime%c;time%c[1] - time%c[0]", LAYER_NAMES[layer], LAYER_NAMES[layer], LAYER_NAMES[layer]),
			200, -5000, 5000
		);

		delete hDxiDtime[layer];
		hDxiDtime[layer] = new TH2F(
			Form("hDxiTot%c", LAYER_NAMES[layer]),
			Form(
				"dxi%c vs dtime%c;time%c[1] - time%c[0] [ps];|xi%c[1] - xi%c[0]|", 
				LAYER_NAMES[layer], LAYER_NAMES[layer], LAYER_NAMES[layer], LAYER_NAMES[layer], LAYER_NAMES[layer], LAYER_NAMES[layer]
			),
			200, -1000, 1000,
			// 320, -0.5, 319.5
			50, -0.5, 49.5
		);
	}

}

void plotClusterLoop(
	const DataFBT2& inData,
	const Float_t (&totRange)[2]
) {
	Double_t minTot = 0.5;

	// coincident event
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.totV[layer]->size() == 0) {
			return;
		}
		if ((*inData.totV[layer])[0] < minTot) {
			return;
		}
	}

	// small misalignment
	Double_t misalignment = abs(
		(Double_t)((*inData.xiV[0])[0] + (*inData.xiV[1])[0])
		- ((Double_t)((*inData.xiV[2])[0]) - MISALIGNMENT_OFFSET) / MISALIGNMENT_SLOPE
	);
	if (misalignment > 5) {
		return;
	}

	// find and count multi-hit layers
	Int_t multiHitLayers = 0;
	Int_t multiHitLayer = -1;
	for (Int_t layer = 0; layer < 3; layer++) {
		// continue for single-hits
		if (inData.totV[layer]->size() == 1) {
			continue;
		}
		if ((*inData.totV[layer])[1] < minTot) {
			continue;
		}
		multiHitLayers++;
		multiHitLayer = layer;
	}

	if (multiHitLayers == 0) {
		for (Int_t layer = 0; layer < 3; layer++) {
			// tot range
			if (!inRange((*inData.totV[layer])[0], totRange)) {
				continue;
			}
			hDxi[layer]->Fill(0);
		}
		return;
	} else if (multiHitLayers == 1) {
		// tot range
		if (!inRange((*inData.totV[multiHitLayer])[0], totRange)) {
			return;
		}

		hDxi[multiHitLayer]->Fill((*inData.xiV[multiHitLayer])[1] - (*inData.xiV[multiHitLayer])[0]);
		hDtime[multiHitLayer]->Fill((*inData.timeV[multiHitLayer])[1] - (*inData.timeV[multiHitLayer])[0]);
		hDxiDtime[multiHitLayer]->Fill(
			(*inData.timeV[multiHitLayer])[1] - (*inData.timeV[multiHitLayer])[0],
			abs((*inData.xiV[multiHitLayer])[1] - (*inData.xiV[multiHitLayer])[0])
		);
	}
}

void plotClusterEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const TString& outName,
	Bool_t saveGraph
) {
	TString graphPath = Form("%s.pdf", outName.Data());
	if (saveGraph) {
		c1->Print(Form("%s[", graphPath.Data()));
	}

	c1->SetGrid();
	c1->SetLogy();
	for (UInt_t layer = 0; layer < 3; layer++) {
		Double_t entries = hDxi[layer]->Integral();
		addStats(hDxi[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", entries), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]), 
			Form("int[0, 0] (single-hit) = %.5g", hDxi[layer]->GetBinContent(hDxi[layer]->FindBin(0)) / entries),
			Form("int[-1, 1] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-1), hDxi[layer]->FindBin(1)) / entries),
			Form("int[-2, 2] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-2), hDxi[layer]->FindBin(2)) / entries),
			Form("int[-3, 3] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-3), hDxi[layer]->FindBin(3)) / entries),
			Form("int[-4, 4] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-4), hDxi[layer]->FindBin(4)) / entries),
			Form("int[-5, 5] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-5), hDxi[layer]->FindBin(5)) / entries),
			Form("int[-10, 10] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-10), hDxi[layer]->FindBin(10)) / entries),
			Form("int[-50, 50] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-50), hDxi[layer]->FindBin(50)) / entries),
			Form("int[-100, 100] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-100), hDxi[layer]->FindBin(100)) / entries),
			Form("int[-319, 319] = %.5g", hDxi[layer]->Integral(hDxi[layer]->FindBin(-319), hDxi[layer]->FindBin(319)) / entries)
		});
		hDxi[layer]->Draw();
		if (saveGraph) {
			c1->Print(graphPath);
			c1->Clear();
		}
	}

	c1->SetLogy(0);
	for (UInt_t layer = 0; layer < 3; layer++) {
		addStats(hDtime[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hDtime[layer]->GetEntries()), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]), 
		});
		hDtime[layer]->Draw();
		if (saveGraph) {
			c1->Print(graphPath);
			c1->Clear();
		}
	}

	c1->SetLogz();
	for (UInt_t layer = 0; layer < 3; layer++) {
		addStats(hDxiDtime[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hDtime[layer]->GetEntries()), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]), 
		});
		hDxiDtime[layer]->Draw();
		if (saveGraph) {
			c1->Print(graphPath);
			c1->Clear();
		}
	}

	if (saveGraph) {
		c1->Print(Form("%s]", graphPath.Data()));
	}
}

void plotCluster(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {0.5, 10},
	Bool_t saveGraph = 1
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotClusterStart(inData, totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotClusterLoop(inData, totRange);
	}

	TString outName = Form("%s_cluster", inData.runNum.Data());
	plotClusterEnd(inData, totRange, outName, saveGraph);
}

