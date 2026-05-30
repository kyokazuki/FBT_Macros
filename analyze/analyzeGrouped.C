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

TCanvas *c1 = nullptr;

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
	plotMultEnd(totRange);
	plotBeamspotEnd(totRange);
	plotPosEnd(totRange);

	// draw graphs
	delete c1;
    c1 = new TCanvas("c1", "c1", 1500, 800);
	c1->Divide(3, 2);

	gStyle->SetOptStat(0);
	for (Int_t i = 0; i < 3; i++) {
		c1->cd(i + 1);
		gPad->SetGrid();
		gPad->SetLogy();
		hMult[i]->Draw();
	}

	c1->cd(4);
	gPad->SetGrid();
	gPad->SetLogz();
	hBeamspot->Draw();

	c1->cd(5);
	gPad->SetGrid();
	gPad->SetLogz();
	hPos->Draw();

	c1->cd(6);
	gPad->SetGrid();
	gPad->SetLogz();
	hPosAligned->Draw();

	c1->Print(graphPath);
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

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeGrouped.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
	);
	analyzeGroupedEnd(inData, totRange, graphPath);
}

