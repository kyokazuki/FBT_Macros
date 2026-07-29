#include <TH2F.h>
#include <TMath.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TH1F* hTotMaxAll = nullptr;
vector<TH2F*> hTotMax(3);

void plotTotMaxStart(const DataFBT2& inData, const Float_t (&totRange)[2]) {
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete hTotMaxAll;
	if (totRange[1] > 10e3) {
		hTotMaxAll = new TH1F(
			"hTotMaxAll",
			"max tot (all layers);tot [ps]",
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
	} else {
		hTotMaxAll = new TH1F(
			"hTotMaxAll",
			"max tot (all layers);tot (scaled)",
			MAX_TOT_BINS, totRange[0], totRange[1]
		);
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hTotMax[layer];
		if (totRange[1] > 10e3) {
			hTotMax[layer] = new TH2F(
				Form("hTotMax%c", LAYER_NAMES[layer]),
				Form("max tot vs xi (%c);xi;tot [ps]", LAYER_NAMES[layer]),
				MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
				MAX_TOT_BINS, totRange[0], totRange[1]
			);
		} else {
			hTotMax[layer] = new TH2F(
				Form("hTotMax%c", LAYER_NAMES[layer]),
				Form("max tot vs xi (%c);xi;tot (scaled)", LAYER_NAMES[layer]),
				MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
				MAX_TOT_BINS, totRange[0], totRange[1]
			);
		}
	}
}

void plotTotMaxLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const Float_t (&posRange)[2]
) {
	// check if max Tot exists and is in range
	Int_t maxTotLayer = inData.getMaxTotLayer(totRange, timingRange);
	if (maxTotLayer == -1) {
		return;
	}

	// check pos
	Float_t posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] - POS_OFFSET) / POS_SLOPE;
	if (!inRange(posAligned, posRange)) {
		return;
	}

	hTotMaxAll->Fill((*inData.totV[maxTotLayer])[0]);
	hTotMax[maxTotLayer]->Fill((*inData.xiV[maxTotLayer])[0], (*inData.totV[maxTotLayer])[0]);
}

void plotTotMaxEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const Float_t (&posRange)[2]
) {
	zoomAxisX(hTotMaxAll, 0, 5);
	addStats(hTotMaxAll, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hTotMaxAll->GetEntries()), 
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
		Form("pos = {%.1f, %.1f}", posRange[0], posRange[1])
	});

	zoomAxisY(hTotMax, 0, 5);
	for (Int_t layer = 0; layer < 3; layer++) {
		addStats(hTotMax[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hTotMax[layer]->GetEntries()), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]), 
			Form("pos = {%.1f, %.1f}", posRange[0], posRange[1])
		});
	}
}

void plotTotMax(
	const TString& inPath, 
	const Float_t (&totRange)[2]		= MAX_TOT_RANGE,
	const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE,
	const Float_t (&posRange)[2]		= MAX_POS_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotTotMaxStart(inData, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTotMaxLoop(inData, totRange, timingRange, posRange);
	}

	plotTotMaxEnd(inData, totRange, timingRange, posRange);
}

