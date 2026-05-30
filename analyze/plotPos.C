#include <iostream>
#include <stdlib.h>

#include <TH2F.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TH2F* hPos = nullptr;
TH2F* hPosAligned = nullptr;
TH1D* hPosAlignedX = nullptr;

void plotPosStart() {
	delete hPos;
	hPos = new TH2F(
		"hPos",
		"(X + Y) vs U;xiX + xiY;xiU",
		LAYER_CHANNELS[1] + LAYER_CHANNELS[0] - 1, MAX_XI_RANGE[0] * 2 - 0.5, LAYER_CHANNELS[0] + LAYER_CHANNELS[1] + 0.5, 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);
	delete hPosAligned;
	hPosAligned = new TH2F(
		"hPosAligned",
		Form("(X + Y) vs U (aligned);(xiX + xiY) - (xiU - %.1f) / %.1f;xiU", POS_INTERCEPT, POS_SLOPE),
		601, -300, 300, 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);
}

void plotPosLoop(const DataFBT2& inData, const Float_t (&totRange)[2]) {
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		if (!inRange((*inData.totV[layer])[0], totRange)) {
			return;
		}
	}

	Long64_t pos = (*inData.xiV[0])[0] + (*inData.xiV[1])[0];
	Long64_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] - POS_INTERCEPT) / POS_SLOPE;

	hPos->Fill(pos, (*inData.xiV[2])[0]);
	hPosAligned->Fill(posAligned, (*inData.xiV[2])[0]);
}

void plotPosEnd(const Float_t (&totRange)[2]) {
	delete hPosAlignedX;
	hPosAlignedX = hPosAligned->ProjectionX("hPosAlignedX");

	Float_t trackedEntries = hPosAlignedX->Integral(
		hPosAlignedX->FindBin(-5),
		hPosAlignedX->FindBin(5)
	);
	Float_t totalEntries = hPosAligned->GetEntries();
	addStats(hPosAligned, {
		Form("entries = %.0f", totalEntries), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]),
		Form("integral[%d, %d] = %.3f", -5, 5, trackedEntries / totalEntries)
	});
}

void plotPos(
	const TString& inPath, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
	}

	plotPosStart();

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		plotPosLoop(inData, totRange);
	}

	plotPosEnd(totRange);
}

