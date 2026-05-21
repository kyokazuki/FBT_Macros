#include <TH2F.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

vector<TH2F*> hXiTot(3);

void plotXiTotEntry(const DataFBT1& inData, const Long64_t entry) {
	// intialize graph
	if (entry == 0) {
		for (Int_t layer = 0; layer < 3; layer++) {
			hXiTot[layer] = new TH2F(
				Form("hXiTot%c", LAYERS[layer]),
				Form("tot vs xi (%c);xi;tot [ps]", LAYERS[layer]),
				MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
				MAX_TOT_BINS, MAX_TOT_RANGE[0], MAX_TOT_RANGE[1]
			);
		}
	}

	if (inData.channelId != 4128) {
		hXiTot[inData.yi]->Fill(inData.xi, inData.tot);
	}

	// adjust graphs
	if (entry == inData.entries - 1) {
		for (Int_t layer = 0; layer < 3; layer++) {
			zoomAxisY(hXiTot[layer], 0, 5);
		}
	}
}

void plotXiTot(const TString& inPath) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		plotXiTotEntry(inData, entry);
	}
}

