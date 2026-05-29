#include <TH2F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"

TH2F* hBeamspot = nullptr;

void plotBeamspotStart(const DataFBT2& inData, const vector<Float_t>& totRange) {
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
	}

	delete hBeamspot;
	hBeamspot = new TH2F(
		"hBeamspot",
		Form("xiY vs xiX (tot = {%.0e, %.0e});xiX;xiY", totRange[0], totRange[1]),
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);
}

void plotBeamspotLoop(const DataFBT2& inData, const vector<Float_t>& totRange) {
	if (!(inData.xiV[0]->size() > 0 && inData.xiV[1]->size() > 0)) {
		return;
	}
	if (!(
		(*inData.totV[0])[0] >= totRange[0] && (*inData.totV[0])[0] < totRange[1] &&
		(*inData.totV[1])[0] >= totRange[0] && (*inData.totV[1])[0] < totRange[1]
	)) {
		return;
	}

	hBeamspot->Fill((*inData.xiV[0])[0], (*inData.xiV[1])[0]);
}

void plotBeamspotEnd() {}

void plotBeamspot(
	const TString& inPath, 
	const vector<Float_t> totRange = MAX_TOT_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotBeamspotStart(inData, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotBeamspotLoop(inData, totRange);
	}

	plotBeamspotEnd();
}

