#include <iostream>
#include <stdlib.h>

#include <TH1F.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TStyle.h>

#include "plotMult.C"

void plotMultHodoLoop(
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

	for (Int_t layer = 0; layer < 3; layer++) {
		Int_t mult = 0;
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			if (inRange((*inData.totV[layer])[hit], totRange)) {
				mult += 1;
			}
		}
		hMult[layer]->Fill(mult);
	}
}

void plotMultHodoEnd(
	const DataFBTHodo& inData, 
	const Float_t (&totRange)[2], 
	const Int_t (&idRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins
) {
	zoomAxisX(hMult, 0, 2);
	for (Int_t layer = 0; layer < 3; layer++) {
		Float_t total = hMult[layer]->GetEntries();
		Float_t effcy = (total - hMult[layer]->GetBinContent(1)) / total;
		addStats(hMult[layer], {
			Form("run%s", getVecString(inData.runNum).Data()),
			Form("entries = %.0f", total), 
			Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]),
			Form("id = {%d, %d}", idRange[0], idRange[1]),
			Form("q = {%.1f, %.1f}", qRange[0], qRange[1]),
			Form("coin = {%s}", getVecString(coins).Data()),
			Form("efficiency = %.4f", effcy)
		});
	}
}

void plotMultHodo(
	const TString& inPath, 
	const Float_t (&totRange)[2]	= MAX_TOT_RANGE,
	const Int_t (&idRange)[2]		= HODO_MAX_ID_RANGE, 
	const Double_t (&qRange)[2]		= HODO_MAX_Q_RANGE,
	const vector<Int_t>& coins		= ALL_COINS
) {
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
	}
	inData.tree->SetBranchStatus("coin", 1);
	inData.tree->SetBranchStatus("fQCal", 1);
	inData.tree->SetBranchStatus("fID", 1);


	plotMultStart();

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultHodoLoop(inData, totRange, idRange, qRange, coins);
	}

	plotMultHodoEnd(inData, totRange, idRange, qRange, coins);
}

