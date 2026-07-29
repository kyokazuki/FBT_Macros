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

void analyzeSinglesStart(const DataFBT1& inData, const Float_t (&totRange)[2], const Long64_t (&timingRange)[2]) {
	plotTotStart(inData, totRange);
	plotTimingStart(inData, totRange, timingRange);
}

void analyzeSinglesLoop(const DataFBT1& inData, Long64_t entry, const Float_t (&totRange)[2], const Long64_t (&timingRange)[2]) {
	plotTotLoop(inData, totRange);
	plotTimingLoop(inData, entry, totRange, timingRange);
}

void analyzeSinglesEnd(const DataFBT1& inData, Bool_t ext, const Float_t (&totRange)[2], const Long64_t (&timingRange)[2], const TString& graphPath) {
	plotTotEnd(inData, totRange);
	plotTimingEnd(inData, totRange, timingRange);

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
	const Float_t (&totRange)[2] = MAX_TOT_RANGE, 
	const Long64_t (&timingRange)[2] = MAX_TIMING_RANGE
) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);

	analyzeSinglesStart(inData, totRange, timingRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeSinglesLoop(inData, entry, totRange, timingRange);
	}

	TString graphPath = Form(
        "%s/%s_analyzeSingles.pdf",
        gSystem->DirName(inPath),
        inData.runNum.Data()
    );
	analyzeSinglesEnd(inData, ext, totRange, timingRange, graphPath);
}

