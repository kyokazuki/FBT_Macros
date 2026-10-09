#include <iostream>
#include <stdlib.h>

#include <TH2F.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TH2F* hMisalignment = nullptr;
TH2F* hMisalignmentShifted = nullptr;
TH1D* hMisalignmentShiftedX = nullptr;

void plotMisalignmentStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete hMisalignment;
	hMisalignment = new TH2F(
		"hMisalignment",
		"(X + Y) vs U;xiX + xiY;xiU",
		LAYER_CHANNELS[1] + LAYER_CHANNELS[0] - 1, 
		MAX_XI_RANGE[0] * 2 - 0.5, 
		LAYER_CHANNELS[0] + LAYER_CHANNELS[1] + 0.5, 
		MAX_XI_BINS, 
		MAX_XI_RANGE[0] - 0.5, 
		MAX_XI_RANGE[1] + 0.5
	);

	delete hMisalignmentShifted;
	hMisalignmentShifted = new TH2F(
		"hMisalignmentShifted",
		Form("(X + Y) vs U (shifted);(xiX + xiY) - (xiU - (%.1f)) / %.1f;xiU", MISALIGNMENT_OFFSET, MISALIGNMENT_SLOPE),
		601, -300, 300, 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);
}

void plotMisalignmentLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	// check all layers
	for (Int_t layer = 0; layer < 3; layer++) {
		// check if is hit
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		// check tot
		if (!inRange((*inData.totV[layer])[0], totRange)) {
			return;
		}
		// check timing
		if (!inRange((*inData.timeV[layer])[0] - (*inData.timeGate)[0], timingRange)) {
			return;
		}
	}

	// Double_t minTot = 0.5;
	//
	// // coincident event
	// for (Int_t layer = 0; layer < 3; layer++) {
	// 	if (inData.totV[layer]->size() == 0) {
	// 		return;
	// 	}
	// 	if ((*inData.totV[layer])[0] < minTot) {
	// 		return;
	// 	}
	// }
	//
	// // single-hit
	// for (Int_t layer = 1; layer < 3; layer++) {
	// 	if (inData.totV[layer]->size() == 1) {
	// 		continue;
	// 	}
	// 	if ((*inData.totV[layer])[1] > minTot) {
	// 		return;
	// 	}
	// }
	//
	// // multi-hit on X
	// if (inData.totV[0]->size() < 2) {
	// 	return;
	// }
	// if ((*inData.totV[0])[1] < minTot) {
	// 	return;
	// }

	Long64_t pos = (*inData.xiV[0])[1] + (*inData.xiV[1])[0];
	Long64_t posAligned = pos - ((*inData.xiV[2])[0] - MISALIGNMENT_OFFSET) / MISALIGNMENT_SLOPE;

	hMisalignment->Fill(pos, (*inData.xiV[2])[0]);
	hMisalignmentShifted->Fill(posAligned, (*inData.xiV[2])[0]);
}

void plotMisalignmentEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	delete hMisalignmentShiftedX;
	hMisalignmentShiftedX = hMisalignmentShifted->ProjectionX("hMisalignmentShiftedX");

	Float_t trackedEntries = hMisalignmentShiftedX->Integral(
		hMisalignmentShiftedX->FindBin(-5),
		hMisalignmentShiftedX->FindBin(5)
	);
	Float_t totalEntries = hMisalignmentShifted->GetEntries();
	addStats(hMisalignmentShifted, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", totalEntries), 
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
		Form("integral[%d, %d] = %.3f", -5, 5, trackedEntries / totalEntries)
	});
}

void plotMisalignment(
	const TString& inPath, 
	const Float_t (&totRange)[2] 		= MAX_TOT_RANGE,
	const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotMisalignmentStart(inData);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMisalignmentLoop(inData, totRange, timingRange);
	}

	plotMisalignmentEnd(inData, totRange, timingRange);
}

