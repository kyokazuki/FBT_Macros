#include <iostream>
#include <stdlib.h>

#include <TH1F.h>
#include <TH2F.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TH1F* hTiming = nullptr;
TH2F* hTimingTot = nullptr;
TH2F* hTimingXi = nullptr;
TH2F* hTimingDxi = nullptr;
TH2F* hDxiTot = nullptr;

void plotTimingStart(const Float_t (&totRange)[2]) {
	delete hTiming;
	hTiming = new TH1F("hTiming",
		"hTiming", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1]
	);

	delete hTimingTot;
	hTimingTot = new TH2F(
		"hTimingTot",
		"tot vs dt;dt [ps];tot [ps]", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1], 
		MAX_TOT_BINS, totRange[0], totRange[1]
	);

	delete hTimingXi;
	hTimingXi = new TH2F(
		"hTimingXi",
		"xi vs dt;dt [ps];xi", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1], 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);

	delete hTimingDxi;
	hTimingDxi = new TH2F(
		"hTimingDxi",
		"dxi vs dt;dt [ps];xi", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1], 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 1 - 0.5, MAX_XI_RANGE[1] - 1 + 0.5
	);

	delete hDxiTot;
	hDxiTot = new TH2F(
		"hDxiTot",
		"tot vs dxi;dxi;tot [ps]", 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 1 - 0.5, MAX_XI_RANGE[1] - 1 + 0.5, 
		MAX_TOT_BINS, totRange[0], totRange[1]
	);
}

void plotTimingLoop(const DataFBT1& inData, Long64_t entry, const Float_t (&totRange)[2]) {
	if (!(inData.channelId == 4128 && inData.energy == 5)) {
		return;
	}

	Int_t xi_ref = inData.xi;
	Long64_t time_ref = inData.time;

	// look both ways
	for (Long64_t dir = -1; dir <= 1; dir = dir + 2) {
		Long64_t row = entry;
		while (1) {
			row = row + dir;
			if (row < 0 || row >= inData.entries) {
				break;
			}
			inData.tree->GetEntry(row);

			Long64_t dt = inData.time - time_ref;

			if ((dir == -1 && dt > MAX_DT_RANGE[1]) || (dir == 1 && dt < MAX_DT_RANGE[0])) {
				continue;
			} else if ((dir == -1 && dt < MAX_DT_RANGE[0]) || (dir == 1 && dt > MAX_DT_RANGE[1])) {
				break;
			}

			if (!(inRange(inData.xi, MAX_XI_RANGE) && inRange(inData.tot, totRange))) {
				continue;
			}

			Int_t dxi = inData.xi - xi_ref;

			hTiming->Fill(dt);
			hTimingTot->Fill(dt, inData.tot);
			hTimingXi->Fill(dt, inData.xi);
			hTimingDxi->Fill(dt, dxi);
			hDxiTot->Fill(dxi, inData.tot);
		}
	}
}

void plotTimingEnd(const Float_t (&totRange)[2]) {
	zoomAxisX(hTiming, 2, 2);

	zoomAxisX(hTimingTot, 2, 2);
	zoomAxisY(hTimingTot, 0, 5);
	addStats(hTimingTot, {
		Form("entries = %.0f", hTimingTot->GetEntries()), 
		Form("tot = {%.0e, %.0e}", totRange[0], totRange[1])	
	});

	zoomAxisX(hTimingXi, 2, 2);
	zoomAxisX(hTimingDxi, 2, 2);
}

void plotTiming(
	const TString& inPath, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE
) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);

	plotTimingStart(totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTimingLoop(inData, entry, totRange);
	}
	
	plotTimingEnd(totRange);
}
