#include <TH1F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

// initialize graphs
vector<TH1F*> hMult(3);

void plotMultStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("totX", 1);
	inData.tree->SetBranchStatus("totY", 1);
	inData.tree->SetBranchStatus("totU", 1);

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hMult[layer];
		hMult[layer] = nullptr;
		hMult[layer] = new TH1F(
			Form("hMult%c", LAYERS[layer]),
			"",
			100, -0.5, 99.5
		);
	}
}

void plotMultLoop(const DataFBT2& inData, const vector<Float_t>& totRange) {
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

void plotMultEnd(const vector<Float_t>& totRange) {
	for (Int_t layer = 0; layer < 3; layer++) {
		Long64_t total = hMult[layer]->GetEntries();
		Float_t effcy = (total - hMult[layer]->GetBinContent(1)) / total;
		hMult[layer]->SetTitle(Form(
			"multiplicity %c (tot = {%.0e, %.0e}) ( effcy = %.4f );multiplicity", 
			LAYERS[layer], totRange[0], totRange[1], effcy
		));
		zoomAxisX(hMult[layer], 0, 2);
	}
}

void plotMult(const TString& inPath, const vector<Float_t> totRange) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotMultStart(inData);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultLoop(inData, totRange);
	}

	plotMultEnd(totRange);
}

