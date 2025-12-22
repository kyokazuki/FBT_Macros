#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include <unistd.h>

void treeFriend(const TString& input_path_1, const TString& input_path_2) {
	TString output_path = input_path_1;
	output_path.ReplaceAll(".root", "_friended.root");
	TFile* input_file_1 = TFile::Open(input_path_1);
	TTree* input_tree_1 = (TTree*)input_file_1->Get("events");
	TFile* input_file_2 = TFile::Open(input_path_2);
	TTree* input_tree_2 = (TTree*)input_file_2->Get("tree");
	TFile* output_file = new TFile(output_path, "RECREATE");
	output_file->cd();
	TTree* output_tree = input_tree_1->CloneTree(0);


	Double_t l1t, l2t, m1t, m2t, s1t, l1q, l2q, m1q, m2q, s1q, rft;
	Long64_t bbtime;
	Int_t scaler[32];
	input_tree_2->SetBranchAddress("l1t", &l1t);
	input_tree_2->SetBranchAddress("l2t", &l2t);
	input_tree_2->SetBranchAddress("m1t", &m1t);
	input_tree_2->SetBranchAddress("m2t", &m2t);
	input_tree_2->SetBranchAddress("s1t", &s1t);
	input_tree_2->SetBranchAddress("l1q", &l1q);
	input_tree_2->SetBranchAddress("l2q", &l2q);
	input_tree_2->SetBranchAddress("m1q", &m1q);
	input_tree_2->SetBranchAddress("m2q", &m2q);
	input_tree_2->SetBranchAddress("s1q", &s1q);
	input_tree_2->SetBranchAddress("rft", &rft);
	input_tree_2->SetBranchAddress("bbtime", &bbtime);
	input_tree_2->SetBranchAddress("scaler", scaler);
	
	output_tree->Branch("l1t", &l1t);
	output_tree->Branch("l2t", &l2t);
	output_tree->Branch("m1t", &m1t);
	output_tree->Branch("m2t", &m2t);
	output_tree->Branch("s1t", &s1t);
	output_tree->Branch("l1q", &l1q);
	output_tree->Branch("l2q", &l2q);
	output_tree->Branch("m1q", &m1q);
	output_tree->Branch("m2q", &m2q);
	output_tree->Branch("s1q", &s1q);
	output_tree->Branch("rft", &rft);
	output_tree->Branch("bbtime", &bbtime);
	output_tree->Branch("scaler", &scaler);

	Long64_t n_entries_1 = input_tree_1->GetEntries();
	Long64_t n_entries_2 = input_tree_2->GetEntries();
	Long64_t loop_entries;

	cout << "Input File 1: " << n_entries_1 << " entries" << endl;
	cout << "Input File 2: " << n_entries_2 << " entries" << endl;
	if (n_entries_1 == n_entries_2) {
		cout << "Gate counts match!" << endl;
	} else {
		cout << "Gate counts don't match!" << endl;
	}

	if (n_entries_1 <= n_entries_2) {
		loop_entries = n_entries_1;
	} else {
		loop_entries = n_entries_2;
	}

	for (Long64_t entry = 0; entry < loop_entries; entry++) {
		input_tree_1->GetEntry(entry);
		input_tree_2->GetEntry(entry);
		output_tree->Fill();
	}

	output_tree->Write();
	output_file->Close();
	input_file_1->Close();
	input_file_2->Close();
}
