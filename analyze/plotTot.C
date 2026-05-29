#include <TH2F.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

vector<TH2F*> hTot(3);

void plotTotStart(const DataFBT1& inData, const vector<Float_t> totRange) {
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hTot[layer];
		hTot[layer] = nullptr;
		if (totRange[1] > 10e3) {
			hTot[layer] = new TH2F(
				Form("hTot%c", LAYERS[layer]),
				Form("tot vs xi (%c);xi;tot [ps]", LAYERS[layer]),
				MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
				MAX_TOT_BINS, totRange[0], totRange[1]
			);
		} else {
			hTot[layer] = new TH2F(
				Form("hTot%c", LAYERS[layer]),
				Form("tot vs xi (%c);xi;tot (scaled)", LAYERS[layer]),
				MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
				MAX_TOT_BINS, totRange[0], totRange[1]
			);
		}
	}
}

void plotTotLoop(const DataFBT1& inData) {
	if (inData.channelId == 4128) {
		return;
	}

	hTot[inData.yi]->Fill(inData.xi, inData.tot);
}

void plotTotEnd() {
	for (Int_t layer = 0; layer < 3; layer++) {
		zoomAxisY(hTot[layer], 0, 5);
	}
}

void plotTot(const TString& inPath, const vector<Float_t> totRange) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);

	plotTotStart(inData, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		plotTotLoop(inData);
	}

	plotTotEnd();
}

