#include <TH1F.h>
#include <TH2F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

// initialize graphs
TH1F* hDt = nullptr;
TH2F* hDtTot = nullptr;
TH2F* hDtXi = nullptr;
TH2F* hDtDxi = nullptr;
TH2F* hDxiTot = nullptr;

void plotDtHitsStart(const DataFBT1& inData, const vector<Float_t>& totRange) {
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);

	delete hDt;
	delete hDtTot;
	delete hDtDxi;
	delete hDxiTot;
	hDt = nullptr;
	hDtTot = nullptr;
	hDtXi = nullptr;
	hDtDxi = nullptr;
	hDxiTot = nullptr;

	hDt = new TH1F("hDt",
		"hDt", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1]
	);
	hDtTot = new TH2F(
		"hDtTot",
		"tot vs dt;dt [ps];tot [ps]", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1], 
		MAX_TOT_BINS, totRange[0], totRange[1]
	);
	hDtXi = new TH2F(
		"hDtXi",
		"xi vs dt;dt [ps];xi", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1], 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5
	);
	hDtDxi = new TH2F(
		"hDtDxi",
		"dxi vs dt;dt [ps];xi", 
		MAX_DT_BINS, MAX_DT_RANGE[0], MAX_DT_RANGE[1], 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 1 - 0.5, MAX_XI_RANGE[1] - 1 + 0.5
	);
	hDxiTot = new TH2F(
		"hDxiTot",
		"tot vs dxi;dxi;tot [ps]", 
		MAX_XI_BINS, MAX_XI_RANGE[0] - 1 - 0.5, MAX_XI_RANGE[1] - 1 + 0.5, 
		MAX_TOT_BINS, totRange[0], totRange[1]
	);
}

void plotDtHitsLoop(const DataFBT1& inData, Long64_t entry, const vector<Float_t>& totRange) {
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

			if (!(inData.xi >= MAX_XI_RANGE[0] && inData.xi <= MAX_XI_RANGE[1])) {
				continue;
			}
			if (!(inData.tot >= totRange[0] && inData.tot <= totRange[1])) {
				continue;
			}
			Int_t dxi = inData.xi - xi_ref;
			hDt->Fill(dt);
			hDtTot->Fill(dt, inData.tot);
			hDtXi->Fill(dt, inData.xi);
			hDtDxi->Fill(dt, dxi);
			hDxiTot->Fill(dxi, inData.tot);
		}
	}
}

void plotDtHitsEnd() {
	zoomAxisX(hDt, 2, 2);
	zoomAxisX(hDtTot, 2, 2);
	zoomAxisY(hDtTot, 0, 5);
	zoomAxisX(hDtXi, 2, 2);
	zoomAxisX(hDtDxi, 2, 2);
}

void plotDtHits(const TString& inPath, const vector<Float_t> totRange) {
	// Set up variables to read from inData.tree
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);

	plotDtHitsStart(inData, totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotDtHitsLoop(inData, entry, totRange);
	}
	
	plotDtHitsEnd();
}
