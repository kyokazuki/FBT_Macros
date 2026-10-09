#include <TH1F.h>
#include <TCanvas.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

vector<TH1F*> hTotZ(3);

void plotTotZStart(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hTotZ[layer];
		hTotZ[layer] = new TH1F(
			Form("hTotZ%c", LAYER_NAMES[layer]),
			Form("tot%c;tot", LAYER_NAMES[layer]),
			200, 0, 4
		);
	}
}

void plotTotZLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2]
) {
	// only plot if all layers have hits
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.totV[layer]->size() == 0) {
			return;
		}
	}

	// check misalignment
	// Float_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] - MISALIGNMENT_OFFSET) / MISALIGNMENT_SLOPE;
	// if (!inRange(posAligned, {-5, 5})) {
	// 	return;
	// }

	// plot if the other layers are within totRange
	for (Int_t layer = 0; layer < 3; layer++) {
		if (!inRange((*inData.totV[(layer + 1) % 3])[0], totRange)) {
			continue;
		}
		if (!inRange((*inData.totV[(layer + 2) % 3])[0], totRange)) {
			continue;
		}

		// check if main layer is single hit
		if (!inRange((*inData.totV[layer])[0], {0.3, 10.0})) {
			return;
		}
		// return if is multi-hit
		if (inData.totV[layer]->size() > 1) {
			if (inRange((*inData.totV[layer])[1], {0.3, 10.0})) {
				continue;
			}
		}
		hTotZ[layer]->Fill((*inData.totV[layer])[0]);
	}
}

void plotTotZEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2],
	const TString& outName
) {
	TCanvas* c1 = nullptr;
	c1 = new TCanvas("c1", "c1", 1500, 400);
	TString graphPath = Form("%s.pdf", outName.Data());
	c1->Print(Form("%s[", graphPath.Data()));
	c1->Divide(3, 1);
	Int_t cell = 1;

	for (Int_t layer = 0; layer < 3; layer++) {
		c1->cd(cell);
		cell++;
		gPad->SetGrid();
		// gPad->SetLogy();

		addStats(hTotZ[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hTotZ[layer]->GetEntries()), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1])
		});
		hTotZ[layer]->Draw();
	}

	c1->Print(graphPath);
	c1->Print(Form("%s]", graphPath.Data()));
	delete c1;
}

void plotTotZ(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2]
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotTotZStart(inData, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTotZLoop(inData, totRange);
	}

	TString outName = Form("%s_totZ", inData.runNum.Data());
	plotTotZEnd(inData, totRange, outName);
}

