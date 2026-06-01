#include <iostream>
#include <stdlib.h>

#include <TSystem.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include "plotTiming.C"
#include "plotTot.C"

TCanvas *cSingles = nullptr;

void analyzeSinglesStart(const Float_t (&totRange)[2]) {
	plotTotStart(totRange);
	plotTimingStart(totRange);
}

void analyzeSinglesLoop(const DataFBT1& inData, Long64_t entry, const Float_t (&totRange)[2]) {
	plotTotLoop(inData, totRange);
	plotTimingLoop(inData, entry, totRange);
}

void analyzeSinglesEnd(const DataFBT1& inData, Bool_t ext, const Float_t (&totRange)[2], const TString& graphPath) {
	plotTotEnd(inData, totRange);
	plotTimingEnd(inData, totRange);

	// draw graphs
	delete cSingles;
    cSingles = new TCanvas("cSingles", "cSingles", 1500, ext ? 800 : 400);

	gStyle->SetOptStat(0);
    if (!ext) {
        cSingles->Divide(3, 1);
    } else {
        cSingles->Divide(3, 2);

        cSingles->cd(4);
        gPad->SetGrid();
        gPad->SetLogz();
        hTimingTot->Draw();

        cSingles->cd(5);
        gPad->SetGrid();
        inData.tree->Draw("time", "xi == 0 && energy == 5");
    }

    for (Int_t i = 0; i < 3; i++) {
        cSingles->cd(i + 1);
        gPad->SetGrid();
        gPad->SetLogz();
        hTot[i]->Draw();
	}

    cSingles->Print(graphPath);	

	cout << "Trigger events: " << inData.tree->GetEntries("channelID==4128 && energy==5") << endl;
}

void analyzeSingles(
	const TString& inPath, 
	Bool_t ext = 1, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE
) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	analyzeSinglesStart(totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeSinglesLoop(inData, entry, totRange);
	}

	TString graphPath = Form(
        "%s/%s_analyzeSingles.pdf",
        gSystem->DirName(inPath),
        inData.runNum[0].Data()
    );
	analyzeSinglesEnd(inData, ext, totRange, graphPath);
}

