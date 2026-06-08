#include <iostream>
#include <stdlib.h>
#include <math.h>

#include <TH2F.h>
#include <TMath.h>

#include "plotTotMax.C"

TH2F* hQMax = nullptr;
TH2F* hTotQ = nullptr;

void plotTotMaxHodoStart(const DataFBTHodo& inData, const Float_t (&totRange)[2]) {
	inData.tree->SetBranchStatus("coin", 1);
	inData.tree->SetBranchStatus("fQCal", 1);
	inData.tree->SetBranchStatus("fTUCal", 1);
	inData.tree->SetBranchStatus("fTDCal", 1);

	plotTotMaxStart(inData, totRange);

	delete hQMax;
	hQMax = new TH2F(
		"hQMax", 
		"max q vs id;id;q",
		HODO_MAX_ID_BINS, HODO_MAX_ID_RANGE[0] - 0.5, HODO_MAX_ID_RANGE[1] + 0.5,
		500, 0, 10
	);
	delete hTotQ;
	if (totRange[1] > 10e3) {
		hTotQ = new TH2F(
			"hTotQ",
			"max tot (all layers) vs max q;q;tot [ps]",
			500, 0, 10,
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
	} else {
		hTotQ = new TH2F(
			"hTotQ",
			"max tot (all layers) vs max q;q;tot (scaled)",
			500, 0, 10,
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
	}
}

void plotTotMaxHodoLoop(
	const DataFBTHodo& inData, 
	const Float_t (&totRange)[2], 
	const Long64_t (&timingRange)[2], 
	const Float_t (&posRange)[2], 
	const Int_t (&idRange)[2], 
	const Double_t (&tRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins
) {
	// check if at least one of the coins is 1
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

	// check for max Q in range
	Int_t maxQId = inData.getMaxQId(idRange, tRange, qRange);
	if (maxQId == -1) {
		return;
	}

	// check if max Tot exists (all 3 layers hit) and is in range
	Int_t maxTotLayer = inData.getMaxTotLayer(totRange, timingRange);
	if (maxTotLayer == -1) {
		return;
	}

	// check pos
	Float_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] + POS_OFFSET) / POS_SLOPE;
	if (!inRange(posAligned, posRange)) {
		return;
	}

	hTotMaxAll->Fill((*inData.totV[maxTotLayer])[0]);
	hTotMax[maxTotLayer]->Fill((*inData.xiV[maxTotLayer])[0], (*inData.totV[maxTotLayer])[0]);
	hQMax->Fill(maxQId, inData.fQCal[inData.getIndex(maxQId)]);
	hTotQ->Fill(inData.fQCal[inData.getIndex(maxQId)], (*inData.totV[maxTotLayer])[0]);
}

void plotTotMaxHodoEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2], 
	const Long64_t (&timingRange)[2], 
	const Float_t (&posRange)[2], 
	const Int_t (&idRange)[2], 
	const Double_t (&tRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins
) {
	zoomAxisX(hTotMaxAll, 0, 5);
	addStats(hTotMaxAll, {
		Form("run%s", getVecString(inData.runNum).Data()),
		Form("entries = %.0f", hTotMaxAll->GetEntries()), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]), 
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
		Form("pos = {%.1f, %.1f}", posRange[0], posRange[1]), 
		Form("id = {%d, %d}", idRange[0], idRange[1]), 
		Form("t = {%.1f, %.1f}", tRange[0], tRange[1]), 
		Form("q = {%.1f, %.1f}", qRange[0], qRange[1]), 
		Form("coin = {%s}", getVecString(coins).Data())
	});

	zoomAxisY(hTotMax, 0, 5);
	for (Int_t layer = 0; layer < 3; layer++) {
		addStats(hTotMax[layer], {
			Form("run%s", getVecString(inData.runNum).Data()), 
			Form("entries = %.0f", hTotMax[layer]->GetEntries()), 
			Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
			Form("pos = {%.1f, %.1f}", posRange[0], posRange[1]), 
			Form("id = {%d, %d}", idRange[0], idRange[1]),
			Form("t = {%.1f, %.1f}", tRange[0], tRange[1]), 
			Form("q = {%.1f, %.1f}", qRange[0], qRange[1]),
			Form("coin = {%s}", getVecString(coins).Data())
		});
	}

	zoomAxisY(hQMax, 0, 5);
	addStats(hQMax, {
		Form("run%s", getVecString(inData.runNum).Data()),
		Form("entries = %.0f", hQMax->GetEntries()), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]), 
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
		Form("pos = {%.1f, %.1f}", posRange[0], posRange[1]), 
		Form("id = {%d, %d}", idRange[0], idRange[1]),
		Form("t = {%.1f, %.1f}", tRange[0], tRange[1]),
		Form("q = {%.1f, %.1f}", qRange[0], qRange[1]),
		Form("coin = {%s}", getVecString(coins).Data())
	});

	zoomAxisAll(hTotQ, 0, 5);
	addStats(hTotQ, {
		Form("run%s", getVecString(inData.runNum).Data()),
		Form("entries = %.0f", hTotQ->GetEntries()), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]), 
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
		Form("pos = {%.1f, %.1f}", posRange[0], posRange[1]), 
		Form("id = {%d, %d}", idRange[0], idRange[1]),
		Form("t = {%.1f, %.1f}", tRange[0], tRange[1]),
		Form("q = {%.1f, %.1f}", qRange[0], qRange[1]),
		Form("coin = {%s}", getVecString(coins).Data())
	});
}

void plotTotMaxHodo(
	const TString& inPath, 
	const Float_t (&totRange)[2]		= MAX_TOT_RANGE,
	const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE,
	const Float_t (&posRange)[2]		= MAX_POS_RANGE,
	const Int_t (&idRange)[2]			= HODO_MAX_ID_RANGE, 
	const Double_t (&tRange)[2]			= HODO_MAX_T_RANGE, 
	const Double_t (&qRange)[2]			= HODO_MAX_Q_RANGE, 
	const vector<Int_t>& coins			= ALL_COINS
) {
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);

	plotTotMaxHodoStart(inData, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTotMaxHodoLoop(inData, totRange, timingRange, posRange, idRange, tRange, qRange, coins);
	}

	plotTotMaxHodoEnd(inData, totRange, timingRange, posRange, idRange, tRange, qRange, coins);
}

