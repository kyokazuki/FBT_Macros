#include <iostream>
#include <stdlib.h>

#include <TH2F.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TH2F* hBeamspot = nullptr;

void plotBeamspotStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete hBeamspot;
	hBeamspot = new TH2F(
		"hBeamspot",
		"xiY vs xiX;xiX;xiY",
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);
}

void plotBeamspotLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const Float_t (&posRange)[2]
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
			continue;
		}
	}

	// check pos
	Float_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] - POS_OFFSET) / POS_SLOPE;
	if (!inRange(posAligned, posRange)) {
		return;
	}

	hBeamspot->Fill((*inData.xiV[0])[0], (*inData.xiV[1])[0]);
}

void plotBeamspotEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const Float_t (&posRange)[2]
) {
	addStats(hBeamspot, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hBeamspot->GetEntries()), 
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
		Form("pos = {%.1f, %.1f}", posRange[0], posRange[1])
	});
}

void plotBeamspot(
	const TString& inPath, 
	const Float_t (&totRange)[2]		= MAX_TOT_RANGE,
	const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE,
	const Float_t (&posRange)[2]		= MAX_POS_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotBeamspotStart(inData);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotBeamspotLoop(inData, totRange, timingRange, posRange);
	}

	plotBeamspotEnd(inData, totRange, timingRange, posRange);
}

