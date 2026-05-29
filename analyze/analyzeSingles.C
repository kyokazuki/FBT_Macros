#include <TSystem.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include "plotDtHits.C"
#include "plotTot.C"

TCanvas *c1 = nullptr;

void analyzeSinglesStart(const DataFBT1& inData, const vector<Float_t>& totRange) {
	plotTotStart(inData, totRange);
	plotDtHitsStart(inData, totRange);
}

void analyzeSinglesLoop(const DataFBT1& inData, Long64_t entry, const vector<Float_t>& totRange) {
	plotTotLoop(inData);
	plotDtHitsLoop(inData, entry, totRange);
}

void analyzeSinglesEnd(const DataFBT1& inData, const TString& graphPath, Bool_t ext) {
	plotTotEnd();
	plotDtHitsEnd();

	// draw graphs
	delete c1;
	c1 = nullptr;
    c1 = new TCanvas("c1", "c1", 1500, ext ? 800 : 400);

	gStyle->SetOptStat("nem");
    if (!ext) {
        c1->Divide(3, 1);
    } else {
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
        hTot[i]->Draw();
    }

    c1->Print(graphPath);	

	cout << "Trigger events: " << inData.tree->GetEntries("channelID==4128 && energy==5") << endl;
}

void analyzeSingles(const TString& inPath, Bool_t ext = 1) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);

	analyzeSinglesStart(inData, MAX_TOT_RANGE);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeSinglesLoop(inData, entry, MAX_TOT_RANGE);
	}

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeSingles.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
    );
	analyzeSinglesEnd(inData, graphPath, ext);
}

