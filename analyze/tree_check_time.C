#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include <unistd.h>

void treeCheckTime(const TString& input_path) {
	TString output_path = input_path;
	output_path.ReplaceAll(".root", "_checked.root");
	TFile* input_file = TFile::Open(input_path);
	TTree* input_tree = (TTree*)input_file->Get("events");
	TFile* output_file = new TFile(output_path, "RECREATE");
	output_file->cd();
	TTree* output_tree = input_tree->CloneTree(0);

	Long64_t bbtime;
	// Int_t scaler[32];
	input_tree->SetBranchAddress("bbtime", &bbtime);
	// input_tree->SetBranchAddress("scaler", scaler);

	vector<Long64_t>* timeX = nullptr;
	vector<Long64_t>* timeY = nullptr;
	vector<Long64_t>* timeU = nullptr;
	input_tree->SetBranchAddress("timeX", &timeX);
	input_tree->SetBranchAddress("timeY", &timeY);
	input_tree->SetBranchAddress("timeU", &timeU);
	
	Long64_t dtime, dbbtime;
	output_tree->Branch("dtime", &dtime);
	output_tree->Branch("dbbtime", &dbbtime);
	output_tree->Branch("timeX", &timeX);
	output_tree->Branch("timeY", &timeY);
	output_tree->Branch("timeU", &timeU);

	Long64_t n_entries = input_tree->GetEntries();

	input_tree->GetEntry(0);
	dtime = 0;
	dbbtime = 0;
	output_tree->Fill();
	Long64_t lastTime = min({(*timeX)[0], (*timeY)[0], (*timeU)[0]});
	Long64_t lastBbtime = bbtime;
	for (Long64_t entry = 1; entry < n_entries; entry++) {
		input_tree->GetEntry(entry);
		dtime = min({(*timeX)[0], (*timeY)[0], (*timeU)[0]}) - lastTime;
		dbbtime = bbtime - lastBbtime;
		output_tree->Fill();
		lastTime = min({(*timeX)[0], (*timeY)[0], (*timeU)[0]});
		lastBbtime = bbtime;
	}

	output_tree->Write();
	output_file->Close();
	input_file->Close();
}
