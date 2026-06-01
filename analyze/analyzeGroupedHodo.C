#include <iostream>
#include <stdlib.h>

#include <TSystem.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>

#include "plotBeamspotHodo.C"
#include "plotMultHodo.C"
#include "plotPosHodo.C"
#include "plotTotMaxHodo.C"

TCanvas *cGrouped = nullptr;

void analyzeGroupedHodoStart(const DataFBTHodo& inData, const Float_t (&totRange)[2]) {
	plotBeamspotHodoStart(inData);
	plotMultHodoStart(inData);
	plotPosHodoStart(inData);
	plotTotMaxHodoStart(inData, {0, totRange[1] / 2});
}

void analyzeGroupedHodoLoop(
	const DataFBTHodo& inData, 
	const Float_t (&totRange)[2],
	const Int_t (&idRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins
) {
	plotBeamspotHodoLoop(inData, totRange, idRange, qRange, coins);
	plotMultHodoLoop(inData, totRange, idRange, qRange, coins);
	plotPosHodoLoop(inData, totRange, idRange, qRange, coins);
	plotTotMaxHodoLoop(inData, {0, totRange[1] / 2}, idRange, qRange, coins);
}

void analyzeGroupedHodoEnd(
	const DataFBTHodo& inData, 
	const Float_t (&totRange)[2],
	const Int_t (&idRange)[2], 
	const Double_t (&qRange)[2], 
	const vector<Int_t>& coins, 
	const TString& graphPath
) {
	plotBeamspotHodoEnd(inData, totRange, idRange, qRange, coins);
	plotMultHodoEnd(inData, totRange, idRange, qRange, coins);
	plotPosHodoEnd(inData, totRange, idRange, qRange, coins);
	plotTotMaxHodoEnd(inData, {0, totRange[1] / 2}, idRange, qRange, coins);

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

void analyzeGroupedHodo(
	const TString& inPath, 
	const Float_t (&totRange)[2]	= {50e3, 1e6},
	const Int_t (&idRange)[2]		= HODO_MAX_ID_RANGE, 
	const Double_t (&qRange)[2]		= HODO_MAX_Q_RANGE, 
	const vector<Int_t>& coins		= ALL_COINS
) {
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);

	analyzeGroupedHodoStart(inData, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeGroupedHodoLoop(inData, totRange, idRange, qRange, coins);
	}

	TString graphPath = Form(
        "%s/%s_analyzeGroupedHodo.pdf",
        gSystem->DirName(inPath),
        inData.runNum[0].Data()
	);
	analyzeGroupedHodoEnd(inData, totRange, idRange, qRange, coins, graphPath);
}

