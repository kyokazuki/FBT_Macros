#include <TSystem.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

#include "plotMult.C"
#include "plotBeamspot.C"
#include "plotPos.C"

void analyzeGrouped(const TString& inPath, const vector<Float_t> totRange = {50000, 1000000}) {
	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeGrouped.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
	);

	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 1);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultEntry(inData, entry, totRange);
		plotBeamspotEntry(inData, entry, totRange);
		plotPosEntry(inData, entry, totRange);
	}

	// draw graphs
	gStyle->SetOptStat(11111111);
	TCanvas *c1 = nullptr;
	c1 = new TCanvas("c1", "c1", 1500, 800);
	c1->Divide(3, 2);

	// multiplicities (efficiencies)
	for (Int_t i = 0; i < 3; i++) {
		c1->cd(i + 1);
		gPad->SetGrid();
		gPad->SetLogy();
		hMult[i]->Draw();
	}

	// beamspot
	c1->cd(4);
	gPad->SetGrid();
	gPad->SetLogz();
	hBeamspot->Draw();

	// alignment
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

