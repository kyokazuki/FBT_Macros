#include <TH1F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

vector<TH1F*> hMult(3);

void plotMultStart() {
	for (Int_t layer = 0; layer < 3; layer++) {
		delete hMult[layer];
		hMult[layer] = nullptr;
		hMult[layer] = new TH1F(
			Form("hMult%c", LAYERS[layer]),
			Form("multiplicity %c;multiplicity", LAYERS[layer]),
			100, -0.5, 99.5
		);
	}
}

void plotMultLoop(const DataFBT2& inData, const Float_t (&totRange)[2]) {
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

void plotMultEnd(const Float_t (&totRange)[2]) {
	for (Int_t layer = 0; layer < 3; layer++) {
		zoomAxisX(hMult[layer], 0, 2);

		Float_t total = hMult[layer]->GetEntries();
		Float_t effcy = (total - hMult[layer]->GetBinContent(1)) / total;
		addStats(hMult[layer], {
			Form("entries = %.0f", total), 
			Form("tot = {%.0e, %.0e}", totRange[0], totRange[1]),
			Form("efficiency = %.4f", effcy)
		});
	}
}

void plotMult(
	const TString& inPath, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
	}

	plotMultStart();

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultLoop(inData, totRange);
	}

	plotMultEnd(totRange);
}

