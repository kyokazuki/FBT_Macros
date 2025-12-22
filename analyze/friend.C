#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include <unistd.h>

void treeFriend(const TString& inputPath1, const TString& inputPath2) {
	TString output_path = inputPath1;
	output_path.ReplaceAll(".root", "_friended.root");
	TFile* inputFile1 = TFile::Open(inputPath1);
	TTree* inputTree1 = (TTree*)inputFile1->Get("events");
	TFile* inputFile2 = TFile::Open(inputPath2);
	TTree* inputTree2 = (TTree*)inputFile2->Get("tree");
	TFile* outputFile = new TFile(output_path, "RECREATE");
	outputFile->cd();
	TTree* outputTree = inputTree1->CloneTree(0);

	vector<Long64_t>* timeX = nullptr;
	vector<Long64_t>* timeY = nullptr;
	vector<Long64_t>* timeU = nullptr;
	inputTree1->SetBranchAddress("timeX", &timeX);
	inputTree1->SetBranchAddress("timeY", &timeY);
	inputTree1->SetBranchAddress("timeU", &timeU);
	Double_t l1t, l2t, m1t, m2t, s1t, l1q, l2q, m1q, m2q, s1q, rft;
	Long64_t bbtime;
	Int_t scaler[32];
	inputTree2->SetBranchAddress("l1t", &l1t);
	inputTree2->SetBranchAddress("l2t", &l2t);
	inputTree2->SetBranchAddress("m1t", &m1t);
	inputTree2->SetBranchAddress("m2t", &m2t);
	inputTree2->SetBranchAddress("s1t", &s1t);
	inputTree2->SetBranchAddress("l1q", &l1q);
	inputTree2->SetBranchAddress("l2q", &l2q);
	inputTree2->SetBranchAddress("m1q", &m1q);
	inputTree2->SetBranchAddress("m2q", &m2q);
	inputTree2->SetBranchAddress("s1q", &s1q);
	inputTree2->SetBranchAddress("rft", &rft);
	inputTree2->SetBranchAddress("bbtime", &bbtime);
	inputTree2->SetBranchAddress("scaler", scaler);
	
	outputTree->Branch("l1t", &l1t);
	outputTree->Branch("l2t", &l2t);
	outputTree->Branch("m1t", &m1t);
	outputTree->Branch("m2t", &m2t);
	outputTree->Branch("s1t", &s1t);
	outputTree->Branch("l1q", &l1q);
	outputTree->Branch("l2q", &l2q);
	outputTree->Branch("m1q", &m1q);
	outputTree->Branch("m2q", &m2q);
	outputTree->Branch("s1q", &s1q);
	outputTree->Branch("rft", &rft);
	outputTree->Branch("bbtime", &bbtime);
	outputTree->Branch("scaler", &scaler);
	Long64_t dtime, dbbtime;
	outputTree->Branch("dtime", &dtime);
	outputTree->Branch("dbbtime", &dbbtime);

	Long64_t entries1 = inputTree1->GetEntries();
	Long64_t entries2 = inputTree2->GetEntries();
	Long64_t loop_entries;

	cout << "Input File 1: " << entries1 << " entries" << endl;
	cout << "Input File 2: " << entries2 << " entries" << endl;
	if (entries1 == entries2) {
		cout << "Gate counts match!" << endl;
	} else {
		cout << "Gate counts don't match!" << endl;
	}

	if (entries1 <= entries2) {
		loop_entries = entries1;
	} else {
		loop_entries = entries2;
	}

	bool nextEntry1, nextEntry2 = 1;
	for (Long64_t entry1 = 0, entry2 = 0; entry1 < entries1 || entry2 < entries2; entry1 += nextEntry1, entry2 += nextEntry2) {
		inputTree1->GetEntry(entry1);
		inputTree2->GetEntry(entry2);
		outputTree->Fill();
	}

	outputTree->Write();
	outputFile->Close();
	inputFile1->Close();
	inputFile2->Close();
}
