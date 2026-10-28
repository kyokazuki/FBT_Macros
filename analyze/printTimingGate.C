#include <iostream>
#include <stdlib.h>
#include <vector>

#include <TCanvas.h>
#include <TH1F.h>
#include <TString.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TCanvas *cTimingGate = nullptr;
TH1F* hTiming = nullptr;
vector<TH1F*> hTimingGate(4, nullptr);
TH1F* hTimingGated = nullptr;

void printTimingGateStart(
	const DataFBT2& inData, 
	const Long64_t (&timingRange)[2]
) {
	// set branches
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete cTimingGate;
    cTimingGate = new TCanvas("cTimingGate", "cTimingGate", 800, 600);

	delete hTiming;
	hTiming = new TH1F(
		"hTiming",
		"; timing [ps]", 
		200, timingRange[0], timingRange[1]
	);

	for (Int_t gateType = 0; gateType < 4; gateType++) {
		delete hTimingGate[gateType];
		hTimingGate[gateType] = new TH1F(
			Form("hTimingGate%d", gateType),
			Form("trigger time %% 5000 = %d; timing [ps]", 1250 * (gateType + 1) % 5000),
			200, timingRange[0], timingRange[1]
		);
	}

	delete hTimingGated;
	hTimingGated = new TH1F(
		"hTimingGated",
		"; timing [ps]", 
		200, timingRange[0], timingRange[1]
	);
}

void printTimingGateLoop(
	const DataFBT2& inData, 
	const Int_t (&xiRange)[2], 
	const Float_t (&totRange)[2], 
	const Long64_t (&timingRange)[2]
) {
	for (UInt_t hit = 0; hit < inData.xiV[0]->size(); hit++) {
		if ((*inData.totV[0])[hit] < 30e3) {
			break;
		}
		// if ((*inData.xiV[0])[hit] != 160) {
		if (!inRange((*inData.xiV[0])[hit], xiRange)) {
			continue;
		}

		hTiming->Fill((*inData.timeV[0])[hit] - (*inData.timeGate)[0]);
		Int_t gateRemain = (*inData.timeGate)[0] % 5000;
		Int_t gateType = (gateRemain / 1250 + 3) % 4;
		hTimingGate[gateType]->Fill((*inData.timeV[0])[hit] - (*inData.timeGate)[0]);
		Long64_t offset = gateType * 2500;
		hTimingGated->Fill((*inData.timeV[0])[hit] + offset - (*inData.timeGate)[0]);
	}
}

void printTimingGateEnd(
	const DataBase& inData, 
	const Int_t (&xiRange)[2], 
	const Float_t (&totRange)[2], 
	const Long64_t (&timingRange)[2],
	const TString& outName
) {
	cTimingGate->SetGrid();
	cTimingGate->Print(Form("%s[", outName.Data()));

	addStats(hTiming, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hTiming->GetEntries()), 
		Form("xiX = {%d, %d}", xiRange[0], xiRange[1]),
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1])	
	});
	hTiming->Draw();
	cTimingGate->Print(outName);

	for (Int_t gateType = 0; gateType < 4; gateType++) {
		addStats(hTimingGate[gateType], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hTimingGate[gateType]->GetEntries()), 
			Form("xiX = {%d, %d}", xiRange[0], xiRange[1]),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1])	
		});
		hTimingGate[gateType]->Draw();
		cTimingGate->Print(outName);
	}

	addStats(hTimingGated, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hTimingGated->GetEntries()), 
		Form("xiX = {%d, %d}", xiRange[0], xiRange[1]),
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1])	
	});
	hTimingGated->Draw();
	cTimingGate->Print(outName);

	cTimingGate->Print(Form("%s]", outName.Data()));
}

void printTimingGate(
	const vector<TString>& inPaths,
	const Int_t (&xiRange)[2] = {160, 160},
	const Float_t (&totRange)[2] = {30e3, 1e6},
	const Long64_t (&timingRange)[2] = {-3535000, -3515000}
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	TString outName;
	outName = Form("%s_timingGate.pdf", inData.runNum.Data());

	printTimingGateStart(inData, timingRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printTimingGateLoop(inData, xiRange, totRange, timingRange);
	}
	
	printTimingGateEnd(inData, xiRange, totRange, timingRange, outName);
}
