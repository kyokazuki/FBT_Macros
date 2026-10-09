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
TH2F* hTimeDiffTot = nullptr;
TH1D *py = nullptr;
TGraph* gPeaks = nullptr;
TF1* fitAll = nullptr;
TGraph* gTimeResTot = nullptr;

void plotTimeResStart(
	const DataFBT2& inData,
	const Float_t (&totRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete c1;
	c1 = new TCanvas("c1", "c1", 800, 600);

	delete hTimeDiffTot;
	hTimeDiffTot = new TH2F(
		"hTimingTot",
		"tot vs time diff;time diff [ps];tot", 
		200, -2000, 2000,
		200, totRange[0], totRange[1]
	);

	delete gTimeResTot;
	gTimeResTot = new TGraph();
}

void plotTimeResLoop(
	const DataFBT2& inData,
	const Float_t (&totRange)[2]
) {
	// select single hits
	for (Int_t layer = 0; layer <= 1; layer++) {			// Y - X
	// for (Int_t layer = 1; layer <= 2; layer++) {			// U - Y
	// for (Int_t layer = 2; layer >= 0; layer = layer - 2) {	// X - U
		if (inData.totV[layer]->size() == 0) {
			return;
		}
		if (!inRange((*inData.totV[layer])[0], totRange)) {
			return;
		}
		// if second hit exists, must has small tot
		if (inData.totV[layer]->size() > 1) {
			if ((*inData.totV[layer])[1] > totRange[0]) {
				return;
			}
		}
	}
	
	// temporary select FBT center
	if (!(
		(*inData.xiV[0])[0] >= 129 && (*inData.xiV[0])[0] <= 192 &&
		(*inData.xiV[1])[0] >= 81 && (*inData.xiV[1])[0] <= 144
	)) {
		return;
	}

	// Y - X
	hTimeDiffTot->Fill(
		(*inData.timeV[1])[0] - (*inData.timeV[0])[0],
		((*inData.totV[1])[0] + (*inData.totV[0])[0]) / 2
	);
	// U - Y
	// hTimeDiffTot->Fill(
	// 	(*inData.timeV[2])[0] - (*inData.timeV[1])[0],
	// 	((*inData.totV[2])[0] + (*inData.totV[1])[0]) / 2
	// );
	// X - U
	// hTimeDiffTot->Fill(
	// 	(*inData.timeV[0])[0] - (*inData.timeV[2])[0],
	// 	((*inData.totV[0])[0] + (*inData.totV[2])[0]) / 2
	// );
}

void plotTimeResEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const TString& outName,
	Bool_t saveGraph
) {
	TString graphPath = Form("%s.pdf", outName.Data());
	if (saveGraph) {
		c1->Print(Form("%s[", graphPath.Data()));
	}

	for (Int_t ybin = 1; ybin <= 200; ybin++) {
		py = hTimeDiffTot->ProjectionX(Form("py_y%d", ybin), ybin, ybin);
		py->Draw();

		if (py->GetEntries() <= 1000) {
			cout << "Not enough entries for bin " << ybin << "!" << endl;
			delete py;
			continue;
		} 

		// fit
		TF1 gaus(Form("g_y%d", ybin), "gaus", -2000, 2000);
		gaus.SetParameters(py->GetMaximum(), 0, py->GetRMS());
		py->Fit(&gaus, "QSR");

		addStats(py, {
			Form("ybin%d", ybin),
			Form("entries = %.0f", py->GetEntries()), 
			Form("amplitude = %.4g", gaus.GetParameter(0)), 
			Form("mean = %.4g", gaus.GetParameter(1)), 
			Form("sigma = %.4g", gaus.GetParameter(2)), 
		});

		// plot to gTimeResTot
		gTimeResTot->SetPoint(
			gTimeResTot->GetN(), 
			hTimeDiffTot->GetYaxis()->GetBinCenter(ybin),
			gaus.GetParameter(2)
		);

		if (saveGraph) {
			c1->SetGrid();
			c1->Print(graphPath);
			c1->Clear();
		}

		delete py;
	}

	addStats(hTimeDiffTot, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hTimeDiffTot->GetEntries()), 
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
	});

	hTimeDiffTot->Draw();

	if (saveGraph) {
		c1->SetLogz();
		c1->SetGrid();
		c1->Print(graphPath);
		c1->Clear();
	}

	gTimeResTot->Draw();
	gTimeResTot->SetTitle("time resolution vs tot;tot;sigma [ps]");

	if (saveGraph) {
		c1->SetLeftMargin(0.15);
		c1->SetGrid();
		c1->Print(graphPath);
		c1->Print(Form("%s]", graphPath.Data()));
	}
}

void plotTimeRes(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {0.5, 10},
	Bool_t saveGraph = 1
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotTimeResStart(inData, totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTimeResLoop(inData, totRange);
	}

	TString outName = Form("%s_timeRes", inData.runNum.Data());
	plotTimeResEnd(inData, totRange, outName, saveGraph);
}

