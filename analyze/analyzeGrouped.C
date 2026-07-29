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
#include "plotTotMax.C"

TCanvas *cGrouped = nullptr;

void analyzeGroupedStart(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2]
) {
	plotBeamspotStart(inData);
	plotMultStart(inData);
	plotTotMaxStart(inData, {0, totRange[1] / 2});
	plotPosStart(inData);
}

void analyzeGroupedLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const Float_t (&posRange)[2]
) {
	plotMultLoop(inData, totRange, timingRange);
	plotBeamspotLoop(inData, totRange, timingRange, posRange);
	plotTotMaxLoop(inData, {0, totRange[1] / 2}, timingRange, posRange);
	plotPosLoop(inData, totRange, timingRange);
}

void analyzeGroupedEnd(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2], 
	const Long64_t (&timingRange)[2],
	const Float_t (&posRange)[2],
	const TString& graphPath
) {
	plotMultEnd(inData, totRange, timingRange);
	plotBeamspotEnd(inData, totRange, timingRange, posRange);
	plotTotMaxEnd(inData, {0, totRange[1] / 2}, timingRange, posRange);
	plotPosEnd(inData, totRange, timingRange);

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
	hTotMaxAll->Draw();

	cGrouped->cd(6);
	gPad->SetGrid();
	gPad->SetLogz();
	hPosAligned->Draw();

	cGrouped->Print(graphPath);
}

void analyzeGrouped(
	const TString& inPath, 
	const Float_t (&totRange)[2]		= {50e3, 1e6},
	const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE,
	const Float_t (&posRange)[2]		= MAX_POS_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	analyzeGroupedStart(inData, totRange);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeGroupedLoop(inData, totRange, timingRange, posRange);
	}

	TString graphPath = Form(
        "%s/%s_analyzeGrouped.pdf",
        gSystem->DirName(inPath),
        inData.runNum.Data()
	);
	analyzeGroupedEnd(inData, totRange, timingRange, posRange, graphPath);
}

