#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TH1F.h>
#include <TH2F.h>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

const Double_t dtRange[2] = {-20e3, 50e3};
const Int_t dtBins = 500;
const Int_t dxiRange[2] = {-320, 320};
const Int_t totRatioBins = 500;
const Int_t dxiSelectRange[2] = {-1, 1};

TH1F* hdt = nullptr;
TH2F* hdt_xi = nullptr;
TH2F* hdt_tot = nullptr;
TH2F* hdt_tot_selected = nullptr;

void plotCrosstalk(const TString& input_path) {
	// hdt = new TH1F("hdt","hdt", dtBins, dtRange[0], dtRange[1]);
	hdt = new TH1F("hdt","hdt", dtBins, -20e3, 50e3);
	hdt_xi = new TH2F("hdt_xi","hdt_xi", dtBins, dtRange[0], dtRange[1], dxiRange[1]-dxiRange[0]+1, dxiRange[0]-0.5, dxiRange[1]+0.5);
	hdt_tot = new TH2F("hdt_tot","hdt_tot", dtBins, dtRange[0], dtRange[1], totRatioBins, 0, 1);
	hdt_tot_selected = new TH2F("hdt_tot_selected","hdt_tot_selected", dtBins, dtRange[0], dtRange[1], totRatioBins, 0, 1);

	TFile* inputFile = TFile::Open(input_path);
	TTree* inputTree = (TTree*)inputFile->Get("events");

	vector<vector <Long64_t>*> 	timeVectors(3, nullptr);
	vector<vector <Float_t>*> 	totVectors(3, nullptr);
	vector<vector <Int_t>*> 	xiVectors(3, nullptr);
	inputTree->SetBranchStatus("*", 0);
	const char* surfaces[3] = {"X","Y","U"};
	for (Int_t i = 0; i < 3; i++) {
		inputTree->SetBranchStatus(Form("time%s", surfaces[i]), 1);
		inputTree->SetBranchStatus(Form("tot%s", surfaces[i]), 1);
		inputTree->SetBranchStatus(Form("xi%s", surfaces[i]), 1);

		inputTree->SetBranchAddress(Form("time%s", surfaces[i]), &timeVectors[i]);
		inputTree->SetBranchAddress(Form("tot%s", surfaces[i]), &totVectors[i]);
		inputTree->SetBranchAddress(Form("xi%s", surfaces[i]), &xiVectors[i]);
	}

	Long64_t events = inputTree->GetEntries();

	for (Long64_t event = 0; event < events; event++) {
		if (event % 10000 == 0 || event == events - 1) {
			cout << "\rEntry: " << event + 1 << "/" << events << flush;
		}

		inputTree->GetEntry(event);

		if (timeVectors[0]->size() <= 1) continue;

		for (UInt_t i = 1; i < timeVectors[0]->size(); i++) {
			Long64_t dt = (*timeVectors[0])[i] - (*timeVectors[0])[0];
			Int_t dxi = (*xiVectors[0])[i] - (*xiVectors[0])[0];
			Float_t totRatio = (Float_t) (*totVectors[0])[i] / (Float_t) (*totVectors[0])[0];

			hdt->Fill(dt);
			hdt_xi->Fill(dt, dxi);
			hdt_tot->Fill(dt, totRatio);

			if (dxi >= dxiSelectRange[0] && dxi <= dxiSelectRange[1]) {
			// if (dxi < dxiSelectRange[0] || dxi > dxiSelectRange[1]) {
				hdt_tot_selected->Fill(dt, totRatio);
			}
		}
	}
	cout << endl;

	inputFile->Close();
}
