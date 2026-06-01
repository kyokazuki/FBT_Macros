#include <iostream>
#include <stdlib.h>

#include <TH2F.h>

#include "plotBeamspot.C"

void plotBeamspotHodoStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("coin", 1);
	inData.tree->SetBranchStatus("fQCal", 1);
	inData.tree->SetBranchStatus("fID", 1);

	plotBeamspotStart(inData);
}

void plotBeamspotHodoLoop(
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

	plotBeamspotLoop(inData, totRange);
}

void plotBeamspotHodoEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2], 
	const Int_t (&idRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins
) {
	addStats(hBeamspot, {
		Form("run%s", getVecString(inData.runNum).Data()),
		Form("entries = %.0f", hBeamspot->GetEntries()), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]),
		Form("id = {%d, %d}", idRange[0], idRange[1]),
		Form("q = {%.1f, %.1f}", qRange[0], qRange[1]),
		Form("coin = {%s}", getVecString(coins).Data())
	});
}

void plotBeamspotHodo(
	const TString& inPath, 
	const Float_t (&totRange)[2]	= MAX_TOT_RANGE, 
	const Int_t (&idRange)[2]		= HODO_MAX_ID_RANGE, 
	const Double_t (&qRange)[2]		= HODO_MAX_Q_RANGE,
	const vector<Int_t>& coins		= ALL_COINS
) {
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);

	plotBeamspotHodoStart(inData);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotBeamspotHodoLoop(inData, totRange, idRange, qRange, coins);
	}

	plotBeamspotHodoEnd(inData, totRange, idRange, qRange, coins);
}

