#include <iostream>
#include <stdlib.h>
#include <math.h>

#include <TH2F.h>
#include <TMath.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

vector<TH2F*> hTotMax(3);
TH1F* hTotMaxAll = nullptr;
TH2F* hQMax = nullptr;
TH2F* hTotQ = nullptr;

void plotTotMaxHodoStart(const DataFBTHodo& inData, const vector<Float_t>& totRange) {
	inData.tree->SetBranchStatus("coin", 1);
	inData.tree->SetBranchStatus("fQCal", 1);
	inData.tree->SetBranchStatus("fID", 1);
	for (UInt_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
	}

	delete hTotMaxAll;
	delete hTotQ;
	if (totRange[1] > 10e3) {
		hTotMaxAll = new TH1F(
			"hTotMaxAll",
			"max tot (all layers);tot [ps]",
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
		hTotQ = new TH2F(
			"hTotQ",
			"max tot (all layers) vs max q;q;tot [ps]",
			500, 0, 10,
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
	} else {
		hTotMaxAll = new TH1F(
			"hTotMaxAll",
			"max tot (all layers);tot (scaled)",
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
		hTotQ = new TH2F(
			"hTotQ",
			"max tot (all layers) vs max q;q;tot (scaled)",
			500, 0, 10,
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
	delete hQMax;
	hQMax = new TH2F(
		"hQMax", 
		"max q vs id;id;q",
		40, 0.5, 40.5,
		500, 0, 10
	);
}

void plotTotMaxHodoLoop(
	const DataFBTHodo& inData, 
	const vector<Float_t>& totRange, 
	const vector<Int_t>& idRange, 
	const vector<Double_t>& qRange,
	const vector<Int_t>& coins
) {
	// check if one of the coins is 1
	Int_t coinOnes = 0;
	for (UInt_t i = 0; i < coins.size(); i++) {
		if (inData.coin[coins[i]] == 1) {
			coinOnes++;
			break;
		}
	}
	if (coinOnes == 0) {
		return;
	}

	// check if max Q exists and is in range
	Int_t maxQId = inData.getMaxQId(idRange);
	if (maxQId == -1) {
		return;
	}
	if (!(inData.fQCal[maxQId - 1] >= qRange[0] && inData.fQCal[maxQId - 1] <= qRange[1])) {
		return;
	}

	// check if max Tot exists and is in range
	Int_t maxTotLayer = inData.getMaxTotLayer();
	if (maxTotLayer == -1) {
		return;
	}
	if (!((*inData.totV[maxTotLayer])[0] >= totRange[0] && (*inData.totV[maxTotLayer])[0] <= totRange[1])) {
		return;
	}

	// fill
	hQMax->Fill(maxQId, inData.fQCal[maxQId - 1]);
	hTotMax[maxTotLayer]->Fill((*inData.xiV[maxTotLayer])[0], (*inData.totV[maxTotLayer])[0]);
	hTotMaxAll->Fill((*inData.totV[maxTotLayer])[0]);
	hTotQ->Fill(inData.fQCal[maxQId - 1], (*inData.totV[maxTotLayer])[0]);
}

void plotTotMaxHodoEnd() {
	zoomAxisX(hTotMaxAll, 0, 5);
	zoomAxisY(hQMax, 0, 5);
	for (Int_t layer = 0; layer < 3; layer++) {
		zoomAxisY(hTotMax[layer], 0, 5);
	}
	zoomAxisAll(hTotQ, 0, 5);
}

void plotTotMaxHodo(
	const TString& inPath, 
	const vector<Float_t>& totRange	= MAX_TOT_RANGE, 
	const vector<Int_t>& idRange	= HODO_MAX_ID_RANGE, 
	const vector<Double_t>& qRange	= HODO_MAX_Q_RANGE, 
	const vector<Int_t>& coins		= ALL_COINS
) {
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);

	plotTotMaxHodoStart(inData, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTotMaxHodoLoop(inData, totRange, idRange, qRange, coins);
	}

	plotTotMaxHodoEnd();
}

