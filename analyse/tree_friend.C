#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include <unistd.h>

void treeFriend(const TString& input_path_1, const TString& input_path_2, const Int_t offset) {
	TString output_path = input_path_1;
	output_path.ReplaceAll(".root", Form("_friended_offset%d.root", offset));
	TFile* input_file_1 = TFile::Open(input_path_1);
	TFile* input_file_2 = TFile::Open(input_path_2);
	TFile* output_file = new TFile(output_path, "RECREATE");
	TTree* input_tree_1 = (TTree*)input_file_1->Get("events");
	TTree* input_tree_2 = (TTree*)input_file_2->Get("mtree");
	TTree* input_tree_3 = (TTree*)input_file_2->Get("stree_offset");
	TTree* output_tree = input_tree_1->CloneTree(0);

	Double_t ult, urt, dlt, drt, ulq, urq, dlq, drq;
	input_tree_2->SetBranchAddress("ult", &ult);
	input_tree_2->SetBranchAddress("urt", &urt);
	input_tree_2->SetBranchAddress("dlt", &dlt);
	input_tree_2->SetBranchAddress("drt", &drt);
	input_tree_2->SetBranchAddress("ulq", &ulq);
	input_tree_2->SetBranchAddress("urq", &urq);
	input_tree_2->SetBranchAddress("dlq", &dlq);
	input_tree_2->SetBranchAddress("drq", &drq);

	Long64_t scaler;
	input_tree_3->SetBranchAddress("scaler32", &scaler);
	
	output_tree->Branch("ult", &ult);
	output_tree->Branch("urt", &urt);
	output_tree->Branch("dlt", &dlt);
	output_tree->Branch("drt", &drt);
	output_tree->Branch("ulq", &ulq);
	output_tree->Branch("urq", &urq);
	output_tree->Branch("dlq", &dlq);
	output_tree->Branch("drq", &drq);
	output_tree->Branch("scaler32", &scaler);

	Long64_t n_entries_1 = input_tree_1->GetEntries();
	Long64_t n_entries_2 = input_tree_2->GetEntries();
	cout << "Input File 1: " << n_entries_1 << "entries" << endl;
	cout << "Input File 2: " << n_entries_2 << "entries" << endl;

	if (n_entries_1 == n_entries_2) {
		for (Long64_t entry = 0; entry < n_entries_1; entry++) {
			input_tree_1->GetEntry(entry);
			input_tree_2->GetEntry(entry + offset);
			output_tree->Fill();
		}
	} else if (n_entries_1 < n_entries_2) {
		for (Long64_t entry = 0; entry < n_entries_1; entry++) {
			input_tree_1->GetEntry(entry);
			input_tree_2->GetEntry(entry + offset);
			output_tree->Fill();
		}
	} else if (n_entries_1 > n_entries_2) {
		for (Long64_t entry = 0; entry < n_entries_2; entry++) {
			input_tree_1->GetEntry(entry + offset);
			input_tree_2->GetEntry(entry);
			output_tree->Fill();
		}
	}

	output_file->cd();
	output_tree->Write();
	output_file->Close();
	input_file_1->Close();
	input_file_2->Close();
}
