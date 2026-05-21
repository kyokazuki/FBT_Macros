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

#include "plotDtHits.C"
#include "plotXiTot.C"

void analyzeSingles(const TString& inPath, bool ext = 1) {
	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeSingles.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
    );

	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotDtHitsEntry(inData, entry);
		plotXiTotEntry(inData, entry);
	}

	// draw graphs
	gStyle->SetOptStat(11111111);
	TCanvas *c1 = nullptr;

	if (ext == 0) {
		c1 = new TCanvas("c1", "c1", 1500, 400);
		c1->Divide(3, 1);
	} else {
		c1 = new TCanvas("c1", "c1", 1500, 800);
		c1->Divide(3, 2);

		c1->cd(4);
		gPad->SetGrid();
		gPad->SetLogz();
		hDtTot->Draw();

		c1->cd(5);
		gPad->SetGrid();
		inData.tree->Draw("time", "xi == 0 && energy == 5");
	}

	for (Int_t i = 0; i < 3; i++) {
		c1->cd(i + 1);
		gPad->SetGrid();
		gPad->SetLogz();
		hXiTot[i]->Draw();
	}

	c1->Print(graphPath);
	
	cout << inData.tree->GetEntries("channelID==4128 && energy==5") << endl;
}

