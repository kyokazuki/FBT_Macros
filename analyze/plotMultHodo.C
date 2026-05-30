#include <TH1F.h>

#include <TSystem.h>
#include <TCanvas.h>
#include <TStyle.h>

#include <iostream>
#include <stdlib.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"
#include "plotMult.C"

// initialize graphs
vector<TH1F*> hMult(3);

void plotMultHodoLoop(const DataFBTHodo& inData, const Float_t (&totRange)[2], Int_t id, const Double_t (&qRange)[2]) {
	if (!(inData.coin[0] == 1 && inData.fQCal[id] >= qRange[0] && inData.fQCal[id] <= qRange[1])) {
		return;
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		Int_t mult = 0;
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			if ((*inData.totV[layer])[hit] >= totRange[0] && (*inData.totV[layer])[hit] <= totRange[1]) {
				mult += 1;
			}
		}
		hMult[layer]->Fill(mult);
	}
}

void plotMultHodoEnd(const Float_t (&totRange)[2], Int_t id, const Double_t (&qRange)[2]) {
	for (Int_t layer = 0; layer < 3; layer++) {
		Long64_t total = hMult[layer]->GetEntries();
		Float_t effcy = (total - hMult[layer]->GetBinContent(1)) / total;
		hMult[layer]->SetTitle(Form(
			"mult%c (tot={%.0e, %.0e}, id=%d, q={%.0e, %.0e}) (effcy=%.4f);multiplicity", 
			LAYERS[layer], totRange[0], totRange[1], id, qRange[0], qRange[1], effcy
		));
		zoomAxisX(hMult[layer], 0, 2);
	}
}

void plotMultHodo(
	const TString& inPath, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE,
	const Int_t (&idRange)[2] = HODO_MAX_ID_RANGE, 
	const Double_t (&qRange)[2] = HODO_MAX_Q_RANGE
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

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultHodoLoop(inData, totRange, id, qRange);
	}

	plotMultHodoEnd(totRange, id, qRange);
}

