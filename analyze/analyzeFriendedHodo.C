#include <TSystem.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include "plotMultHodo.C"

void analyzeFriendedHodo(
	const TString& inPath, 
	const vector<Float_t> totRange, 
	const Int_t id, 
	const vector<Double_t> qRange
) {
	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeFriendedHodo.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
	);

	// Set up variables to read from inData.tree
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);

	plotMultHodoStart(inData);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultHodoLoop(inData, totRange, id, qRange);
	}

	plotMultHodoEnd(totRange, id, qRange);

	// draw graphs
	gStyle->SetOptStat("nem");
	TCanvas* c1 = new TCanvas("c1", "c1", 1500, 800);

	// draw graphs
	c1->Divide(3, 2);

	c1->cd(1);
	gPad->SetGrid();
	gPad->SetLogz();
	inData.tree->Draw("fQCal:fID>>(40, -0.5, 39.5, 200, 0, 40)", "", "colz");

	// multiplicities (efficiencies)
	for (Int_t i = 0; i < 3; i++) {
		c1->cd(i + 4);
		gPad->SetGrid();
		gPad->SetLogy();
		hMult[i]->Draw();
	}

	c1->Print(graphPath);
}

