#include <TH1F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

// initialize graphs
vector<TH1F*> hMult(3);

void plotMultEntry(const DataFBT2& inData, Long64_t entry, const vector<Float_t> totRange) {
	// intialize graph
	if (entry == 0) {
		for (Int_t layer = 0; layer < 3; layer++) {
			hMult[layer] = new TH1F(
				Form("hMult%c", LAYERS[layer]),
				"",
				100, -0.5, 99.5
			);
		}
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

	// calculate efficiency
	if (entry == inData.entries - 1) {
		for (Int_t layer = 0; layer < 3; layer++) {
			Float_t effcy = (inData.entries - hMult[layer]->GetBinContent(1)) / inData.entries;
			hMult[layer]->SetTitle(Form(
				"multiplicity %c (tot = {%.0e, %.0e}) (efficiency = %.4f);multiplicity", 
				LAYERS[layer], totRange[0], totRange[1], effcy
			));
			zoomAxisX(hMult[layer], 0, 2);
		}
	}
}

void plotMult(const TString& inPath, const vector<Float_t> totRange) {
	// Set up variables to read from inData.tree
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("totX", 1);
	inData.tree->SetBranchStatus("totY", 1);
	inData.tree->SetBranchStatus("totU", 1);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		plotMultEntry(inData, entry, totRange);
	}
}

