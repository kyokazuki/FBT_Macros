#include <iostream>
#include <stdlib.h>

#include <TSystem.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include "plotBeamspot.C"
#include "plotMult.C"
#include "plotPos.C"

TCanvas *cGrouped = nullptr;

void analyzeGroupedStart() {
	plotBeamspotStart();
	plotMultStart();
	plotPosStart();
}

void analyzeGroupedLoop(const DataFBT2& inData, const Float_t (&totRange)[2]) {
	plotMultLoop(inData, totRange);
	plotBeamspotLoop(inData, totRange);
	plotPosLoop(inData, totRange);
}

void analyzeGroupedEnd(const DataFBT2& inData, const Float_t (&totRange)[2], const TString& graphPath) {
	plotMultEnd(inData, totRange);
	plotBeamspotEnd(inData, totRange);
	plotPosEnd(inData, totRange);

	// draw graphs
	delete cGrouped;
    cGrouped = new TCanvas("cGrouped", "cGrouped", 1500, 800);
	cGrouped->Divide(3, 2);

	gStyle->SetOptStat(0);
	for (Int_t i = 0; i < 3; i++) {
		cGrouped->cd(i + 1);
		gPad->SetGrid();
		gPad->SetLogy();
		hMult[i]->Draw();
	}

	cGrouped->cd(4);
	gPad->SetGrid();
	gPad->SetLogz();
	hBeamspot->Draw();

	cGrouped->cd(5);
	gPad->SetGrid();
	gPad->SetLogz();
	hPos->Draw();

	cGrouped->cd(6);
	gPad->SetGrid();
	gPad->SetLogz();
	hPosAligned->Draw();

	cGrouped->Print(graphPath);
}

void analyzeGrouped(const TString& inPath, const Float_t (&totRange)[2] = {50e3, 1e6}) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
	}

	analyzeGroupedStart();

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeGroupedLoop(inData, totRange);
	}

	TString graphPath = Form(
        "%s/%s_analyzeGrouped.pdf",
        gSystem->DirName(inPath),
        inData.runNum[0].Data()
	);
	analyzeGroupedEnd(inData, totRange, graphPath);
}

