#include <TH2F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
// #include "utils/zoomAxis.C"

TH2F* hPos = nullptr;
TH2F* hPosAligned = nullptr;
TH1D* hPosAlignedX = nullptr;

void plotPosEntry(const DataFBT2& inData, Long64_t entry, const vector<Float_t> totRange) {
	if (entry == 0) {
		hPos = new TH2F(
			"hPos",
			Form("(X + Y) vs U (tot = {%.0e, %.0e});xiX + xiY;xiU", totRange[0], totRange[1]),
			LAYER_CHANNELS[1] + LAYER_CHANNELS[0] - 1, MAX_XI_RANGE[0] * 2 - 0.5, LAYER_CHANNELS[0] + LAYER_CHANNELS[1] + 0.5, 
			MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
		);
		hPosAligned = new TH2F(
			"hPosAligned",
			";(xiX + xiY) - (xiU + 39.14285) / 0.73142;xiU",
			// ";(xiX + xiY) - (xiU + 70) / 0.83;xiU",
			601, -300, 300, 
			MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
		);
	}

	if (inData.xiV[0]->size() > 0 && inData.xiV[1]->size() > 0 && inData.xiV[2]->size() > 0) {
		if (
			(*inData.totV[0])[0] >= totRange[0] && (*inData.totV[0])[0] < totRange[1] &&
			(*inData.totV[1])[0] >= totRange[0] && (*inData.totV[1])[0] < totRange[1] &&
			(*inData.totV[2])[0] >= totRange[0] && (*inData.totV[2])[0] < totRange[1]
		) {
			Long64_t pos = (*inData.xiV[0])[0] + (*inData.xiV[1])[0];
			Long64_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] + 39.14285)/0.73142;
			// Long64_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] + 70)/0.83; // FDC1 test cosmic ray

			hPos->Fill(pos, (*inData.xiV[2])[0]);
			hPosAligned->Fill(posAligned, (*inData.xiV[2])[0]);
		}
	}

	if (entry == inData.entries - 1) {
		hPosAlignedX = hPosAligned->ProjectionX("hPosAlignedX");

		Float_t alignedEvents = hPosAlignedX->Integral(
			hPosAlignedX->FindBin(-5),
			hPosAlignedX->FindBin(5)
		);
		Float_t totalEvents = hPosAlignedX->GetEntries();

		hPosAligned->SetTitle(Form(
			"(X + Y) vs U (aligned) (tot = {%.0e, %.0e}) (Integral[-5, 5] = %.3f)", 
			totRange[0], totRange[1], alignedEvents / totalEvents
		));
	}
}

void plotPos(const TString& inPath, const vector<Float_t> totRange) {
	// Set up variables to read from inData.tree
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
	}

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		plotPosEntry(inData, entry, totRange);
	}
}

