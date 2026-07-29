#include <iostream>
#include <stdlib.h>
#include <vector>

#include <TTree.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TStyle.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"

vector<TH2F*> hBeamspotU(320);

void printBeamspotUStart(const DataFBT2& inData) {
	// set branches
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	// initiate histograms
	for (Int_t i = 0; i < 320; i++) {
		delete hBeamspotU[i];
		hBeamspotU[i] = new TH2F(
			Form("hBeamspotU_u%d", i + 1),
			Form("xiY vs xiX (xiU = %d);xiX;xiY", i + 1),
			320, 0.5, 320.5, 
			224, 0.5, 224.5
		);
	}
}

void printBeamspotULoop(const DataFBT2& inData) {
	// check all layers
	for (Int_t layer = 0; layer < 3; layer++) {
		// check if is hit
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		// check tot
		if (!inRange((*inData.totV[layer])[0], {(Float_t)1, (Float_t)10})) {
			return;
		}
		// remove multi hits
		if ((*inData.totV[layer])[1] > 0.5) {
			return;
		}
	}

	hBeamspotU[(*inData.xiV[2])[0] - 1]->Fill((*inData.xiV[0])[0], (*inData.xiV[1])[0]);
}

void printBeamspotUEnd(const DataBase& inData) {
	gStyle->SetOptStat(0);

	// output position pdf
	TCanvas *cBeamspotU = new TCanvas("cBeamspotU", "cBeamspotU", 800, 600);
	TString graphPath = Form("%s_beamspotU.pdf", inData.runNum.Data());
	cBeamspotU->Print(Form("%s[", graphPath.Data()));
	for (Int_t i = 0; i < 320; i++) {
		cBeamspotU->SetGrid();
		cBeamspotU->SetLogz();
		hBeamspotU[i]->Draw();
		cBeamspotU->Print(graphPath);
	}
	cBeamspotU->Print(Form("%s]", graphPath.Data()));
	cBeamspotU->Clear();
}

void printBeamspotU(const TString& inPath) {
	// load data
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	printBeamspotUStart(inData);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printBeamspotULoop(inData);
	}

	printBeamspotUEnd(inData);
}

