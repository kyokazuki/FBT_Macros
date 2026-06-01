#include <iostream>
#include <stdlib.h>

#include <TH2F.h>

#include "plotPos.C"

void plotPosHodoLoop(
	const DataFBTHodo& inData, 
	const Float_t (&totRange)[2], 
	const Int_t (&idRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins
) {
	// check if at least one of the coins is 1
	if (inData.getCoinOnes(coins) == 0) {
		return;
	}
	// check if max Q exists in id range and is in q range
	if (inData.getMaxQId(idRange, qRange) == -1) {
		return;
	}
	// check if all layers are hit and are in range
	if (!(
		inData.totV[0]->size() > 0 &&
		inData.totV[1]->size() > 0 && 
		inData.totV[2]->size() > 0
	)) {
		return;
	}
	if (!(
		inRange((*inData.totV[0])[0], totRange) &&
		inRange((*inData.totV[1])[0], totRange) &&
		inRange((*inData.totV[2])[0], totRange)
	)) {
		return;
	}

	Long64_t pos = (*inData.xiV[0])[0] + (*inData.xiV[1])[0];
	Long64_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] - POS_INTERCEPT) / POS_SLOPE;

	hPos->Fill(pos, (*inData.xiV[2])[0]);
	hPosAligned->Fill(posAligned, (*inData.xiV[2])[0]);
}

void plotPosHodoEnd(
	const DataFBTHodo& inData, 
	const Float_t (&totRange)[2], 
	const Int_t (&idRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins
) {
	delete hPosAlignedX;
	hPosAlignedX = hPosAligned->ProjectionX("hPosAlignedX");

	Float_t trackedEntries = hPosAlignedX->Integral(
		hPosAlignedX->FindBin(-5),
		hPosAlignedX->FindBin(5)
	);
	Float_t totalEntries = hPosAligned->GetEntries();
	addStats(hPosAligned, {
		Form("run%s", getVecString(inData.runNum).Data()),
		Form("entries = %.0f", totalEntries), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]),
		Form("id = {%d, %d}", idRange[0], idRange[1]),
		Form("q = {%.1f, %.1f}", qRange[0], qRange[1]),
		Form("coin = {%s}", getVecString(coins).Data()),
		Form("integral[%d, %d] = %.3f", -5, 5, trackedEntries / totalEntries)
	});
}

void plotPosHodo(
	const TString& inPath, 
	const Float_t (&totRange)[2]	= MAX_TOT_RANGE, 
	const Int_t (&idRange)[2]		= HODO_MAX_ID_RANGE, 
	const Double_t (&qRange)[2]		= HODO_MAX_Q_RANGE,
	const vector<Int_t>& coins		= ALL_COINS
) {
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);
	for (UInt_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
	}
	inData.tree->SetBranchStatus("fID", 1);
	inData.tree->SetBranchStatus("fQCal", 1);

	plotPosStart();

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotPosHodoLoop(inData, totRange, idRange, qRange, coins);
	}

	plotPosHodoEnd(inData, totRange, idRange, qRange, coins);
}

