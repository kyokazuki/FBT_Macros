#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <TTree.h>
#include <TH2F.h>
#include <TH1.h>
#include <TF1.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TMath.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

vector<TH2F*> hTimingXi(3);

void printTimingXiStart(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	gROOT->SetBatch(kTRUE);

	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hTimingXi[layer];
		hTimingXi[layer] = new TH2F(
			Form("hTimingXi%c", LAYER_NAMES[layer]),
			Form(";timing [ps];xi%c", LAYER_NAMES[layer]),
			300, timingRange[0], timingRange[1], 
			320, 0.5, 320.5
		);
	}
}

void printTimingXiLoop(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	// plot single-hits
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		if (!inRange((*inData.totV[layer])[0], totRange)) {
			return;
		}
		if (inData.xiV[layer]->size() > 1) {
			if (inRange((*inData.totV[layer])[1], totRange)) {
				return;
			}
		}
	}

	// filter out irrelevant hits
	if (!inRange(
		((*inData.xiV[0])[0] + (*inData.xiV[1])[0]) - ((*inData.xiV[2])[0] - MISALIGNMENT_OFFSET) / MISALIGNMENT_SLOPE, {-5, 5}
	)) {
		return;
	}

	// check timing
	Long64_t timing[3];
	for (Int_t layer = 0; layer < 3; layer++) {
		timing[layer] = (*inData.timeV[layer])[0] - (*inData.timeGate)[0];
		if (!inRange(timing[layer], timingRange)) {
			return;
		}
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		hTimingXi[layer]->Fill(
			timing[layer],
			(*inData.xiV[layer])[0]
		);
	}
}

void printTimingXiEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const TString& outName
) {
	TCanvas* c1 = nullptr;
	c1 = new TCanvas("c1", "c1", 800, 600);
	TString graphPath = Form("%s.pdf", outName.Data());
	c1->Print(Form("%s[", graphPath.Data()));
	c1->SetLogz();
	c1->SetGrid();

	for (Int_t layer = 0; layer < 3; layer++) {
		addStats(hTimingXi[layer], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTimingXi[layer]->GetEntries()), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
			"misalignment = {-5, 5}"
		});
		hTimingXi[layer]->Draw();
		c1->Print(graphPath);
	}

	c1->Print(Form("%s]", graphPath.Data()));
	delete c1;
}

void printTimingXi(
	const vector<TString>& inPaths, 
	const Float_t (&totRange)[2] = {30e3, 1e6},
	const Long64_t (&timingRange)[2] = {-3542000, -3488000}
	// const Long64_t (&timingRange)[2] = {-16000, 38000}
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printTimingXiStart(inData, totRange, timingRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printTimingXiLoop(inData, totRange, timingRange);
	}

	TString outName = Form("%s_timingXi", inData.runNum.Data());
	printTimingXiEnd(inData, totRange, timingRange, outName);
}
