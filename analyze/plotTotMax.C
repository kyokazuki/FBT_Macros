#include <TH2F.h>
#include <TMath.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TH2F* hTotMaxAll = nullptr;
vector<TH2F*> hTotMax(3);

void plotTotMaxStart(const vector<Float_t>& totRange) {
	delete hTotMaxAll;
	if (totRange[1] > 10e3) {
		hTotMaxAll = new TH2F(
			"hTotMaxAll",
			"max tot vs xi (all layers);xi;tot [ps]",
			MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
	} else {
		hTotMaxAll = new TH2F(
			"hTotMaxAll",
			"max tot vs xi (all layers);xi;tot (scaled)",
			MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
	}
	for (Int_t layer = 0; layer < 3; layer++) {
		delete hTotMax[layer];
		if (totRange[1] > 10e3) {
			hTotMax[layer] = new TH2F(
				Form("hTotMax%c", LAYERS[layer]),
				Form("max tot vs xi (%c);xi;tot [ps]", LAYERS[layer]),
				MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
				MAX_TOT_BINS, totRange[0], totRange[1]
			);
		} else {
			hTotMax[layer] = new TH2F(
				Form("hTotMax%c", LAYERS[layer]),
				Form("max tot vs xi (%c);xi;tot (scaled)", LAYERS[layer]),
				MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
				MAX_TOT_BINS, totRange[0], totRange[1]
			);
		}
	}
}

void plotTotMaxLoop(const DataFBT2& inData, const vector<Float_t>& totRange) {
	Int_t maxTotLayer = inData.getMaxTotLayer();
	if (maxTotLayer == -1) {
		return;
	}
	if (!((*inData.totV[maxTotLayer])[0] >= totRange[0] && (*inData.totV[maxTotLayer])[0] <= totRange[1])) {
		return;
	}

	hTotMax[maxTotLayer]->Fill((*inData.xiV[maxTotLayer])[0], (*inData.totV[maxTotLayer])[0]);
	hTotMaxAll->Fill((*inData.xiV[maxTotLayer])[0], (*inData.totV[maxTotLayer])[0]);
}

void plotTotMaxEnd() {
	zoomAxisY(hTotMaxAll, 0, 5);
	for (Int_t layer = 0; layer < 3; layer++) {
		zoomAxisY(hTotMax[layer], 0, 5);
	}
}

void plotTotMax(const TString& inPath, const vector<Float_t>& totRange = MAX_TOT_RANGE) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
	}

	plotTotMaxStart(totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTotMaxLoop(inData, totRange);
	}

	plotTotMaxEnd();
}

