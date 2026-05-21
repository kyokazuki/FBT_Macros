#include <TH2F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
// #include "utils/zoomAxis.C"

TH2F* hBeamspot = nullptr;

void plotBeamspotEntry(const DataFBT2& inData, const Long64_t entry, const vector<Float_t> totRange) {
	// intialize graph
	if (entry == 0) {
		hBeamspot = new TH2F(
			"hBeamspot",
			Form("xiY vs xiX (tot = {%.0e, %.0e});xiX;xiY", totRange[0], totRange[1]),
			MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
			MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
		);
	}

	if (inData.xiV[0]->size() > 0 && inData.xiV[1]->size() > 0) {
		if (
			(*inData.totV[0])[0] >= totRange[0] && (*inData.totV[0])[0] < totRange[1] &&
			(*inData.totV[1])[0] >= totRange[0] && (*inData.totV[1])[0] < totRange[1]
		) {
			hBeamspot->Fill((*inData.xiV[0])[0], (*inData.xiV[1])[0]);
		}
	}
}

void plotBeamspot(const TString& inPath, const vector<Float_t> totRange) {
	// Set up variables to read from inData.tree
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
	}

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		plotBeamspotEntry(inData, entry, totRange);
	}
}

