#include <iostream>
#include <stdlib.h>

#include <TSystem.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include "plotTiming.C"
#include "plotTot.C"

TCanvas *c1 = nullptr;

void analyzeSinglesStart(const Float_t (&totRange)[2]) {
	plotTotStart(totRange);
	plotTimingStart(totRange);
}

void analyzeSinglesLoop(const DataFBT1& inData, Long64_t entry, const Float_t (&totRange)[2]) {
	plotTotLoop(inData);
	plotTimingLoop(inData, entry, totRange);
}

void analyzeSinglesEnd(const DataFBT1& inData, const Float_t (&totRange)[2], const TString& graphPath, Bool_t ext) {
	plotTotEnd(totRange);
	plotTimingEnd(totRange);

	// draw graphs
	delete c1;
    c1 = new TCanvas("c1", "c1", 1500, ext ? 800 : 400);

	gStyle->SetOptStat(0);
    if (!ext) {
        c1->Divide(3, 1);
    } else {
        c1->Divide(3, 2);

        c1->cd(4);
        gPad->SetGrid();
        gPad->SetLogz();
        hTimingTot->Draw();

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

void analyzeSingles(const TString& inPath, Bool_t ext = 1, const Float_t (&totRange)[2] = MAX_TOT_RANGE) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	analyzeSinglesStart(totRange);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeSinglesLoop(inData, entry, totRange);
	}

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeSingles.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
    );
	analyzeSinglesEnd(inData, totRange, graphPath, ext);
}

