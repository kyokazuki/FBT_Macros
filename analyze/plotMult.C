#include <TH1F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

vector<TH1F*> hMult(3, nullptr);

void plotMultStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hMult[layer];
		hMult[layer] = new TH1F(
			Form("hMult%c", LAYER_NAMES[layer]),
			Form("multiplicity %c;multiplicity", LAYER_NAMES[layer]),
			100, -0.5, 99.5
		);
	}
}

void plotMultLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		Int_t mult = 0;
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			// check timing
			if (!inRange((*inData.timeV[layer])[hit] - (*inData.timeGate)[0], timingRange)) {
				continue;
			}
			// check tot
			if (!inRange((*inData.totV[layer])[hit], totRange)) {
				continue;
			}

			mult += 1;
		}
		hMult[layer]->Fill(mult);
	}
}

void plotMultEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	zoomAxisX(hMult, 0, 2);
	for (Int_t layer = 0; layer < 3; layer++) {
		Float_t total = hMult[layer]->GetEntries();
		Float_t effcy = (total - hMult[layer]->GetBinContent(1)) / total;
		addStats(hMult[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", total), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
			Form("efficiency = %.4f", effcy)
		});
	}
}

void plotMult(
	const TString& inPath, 
	const Float_t (&totRange)[2]		= MAX_TOT_RANGE,
	const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotMultStart(inData);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultLoop(inData, totRange, timingRange);
	}

	plotMultEnd(inData, totRange, timingRange);
}

