#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TString.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <vector>

#include <iostream>
#include <stdlib.h>

#include "utils/printProgress.C"

int processRate(const TString& inputPath) {
	const bool SAVE_GRAPH = 1;
	const Int_t windowHalfWidth = 100;
	const Int_t windowFullWidth = 2 * windowHalfWidth + 1;

	// load input and output files
	if (gSystem->AccessPathName(inputPath)) {
		cout << inputPath << " does not exist!" << endl;
		return 2;
	}
	TFile* inputFile = TFile::Open(inputPath);
	TTree* inputTree = (TTree*)inputFile->Get("events");
	cout << "Rating " << inputPath << endl;

	cout << "Loading tree..." << endl;
	TString outputPath = inputPath;
	outputPath.ReplaceAll(".root", "_rated.root");
	TFile* outputFile = new TFile(outputPath, "RECREATE");
	outputFile->cd();
	TTree* outputTree = inputTree->CloneTree();

	// load input tree
	vector<Long64_t>* timeGate = nullptr;
	Int_t scaler[32];
	inputTree->SetBranchAddress("timeGate", &timeGate);
	inputTree->SetBranchAddress("scaler", scaler);

	inputTree->SetBranchStatus("*", 0);
	inputTree->SetBranchStatus("timeGate", 1);
	inputTree->SetBranchStatus("scaler", 1);

	// preload
	Long64_t entries = inputTree->GetEntries();
	vector <Long64_t> timeGateEntries(entries);
	vector <Int_t> scalerEntries(entries);
	for (Long64_t i = 0; i < entries; i++) {
		inputTree->GetEntry(i);
		timeGateEntries[i]  = (*timeGate)[0];
		scalerEntries[i] = scaler[10];
	}

	// load branches for output tree
	Float_t rate;
	TBranch* rateBranch = outputTree->Branch("rate", &rate);

	// fill rate banch
	for (Int_t entry = 0; entry < entries; entry++) {
		printProgress(entry, entries);

		if (entry < windowHalfWidth || entry >= entries - windowHalfWidth) {
			rate = 0.f;
		} else {
			Long64_t dt = timeGateEntries[entry + windowHalfWidth] - timeGateEntries[entry - windowHalfWidth];
			Int_t    dc = scalerEntries[entry + windowHalfWidth] - scalerEntries[entry - windowHalfWidth];
			rate = 1e12 * (Float_t)dc / (Float_t)dt;
		}
		rateBranch->Fill();
	}
	cout << endl;

	// save rate graph
	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	if (SAVE_GRAPH) {
		TString runNumber = TString(gSystem->BaseName(inputPath))(0,4);
		TString graphPath = Form("%s_rate.pdf", runNumber.Data());
		outputTree->Draw("rate:timeGate", "Entry$ % 100 == 0");
		c1->Print(graphPath);
	}
	delete c1;

	// Save output file
	outputTree->Write();
	outputFile->Close();
	inputFile->Close();
	cout << "Saved to " << outputPath << endl;

	return 0;
}
