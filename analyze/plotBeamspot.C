#include <iostream>
#include <stdlib.h>

#include <TH2F.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TH2F* hBeamspot = nullptr;

void plotBeamspotStart(const DataFBT2& inData) {
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
	}

	delete hBeamspot;
	hBeamspot = new TH2F(
		"hBeamspot",
		"xiY vs xiX;xiX;xiY",
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);
}

void plotBeamspotLoop(const DataFBT2& inData, const Float_t (&totRange)[2]) {
	// check if totX and totY exist and is in range
	if (!(inData.totV[0]->size() > 0 && inData.totV[1]->size() > 0)) {
		return;
	}
	if (!(
		inRange((*inData.totV[0])[0], totRange) && 
		inRange((*inData.totV[1])[0], totRange)
	)) {
		return;
	}

	hBeamspot->Fill((*inData.xiV[0])[0], (*inData.xiV[1])[0]);
}

void plotBeamspotEnd(const DataBase& inData, const Float_t (&totRange)[2]) {
	addStats(hBeamspot, {
		Form("run%s", getVecString(inData.runNum).Data()),
		Form("entries = %.0f", hBeamspot->GetEntries()), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1])
	});
}

void plotBeamspot(
	const TString& inPath, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotBeamspotStart(inData);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotBeamspotLoop(inData, totRange);
	}

	plotBeamspotEnd(inData, totRange);
}

