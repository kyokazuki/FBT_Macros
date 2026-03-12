#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "utils/printProgress.C"

const Float_t TOT_TARGET = 120000;

void processScale(const TString& inputPath, const char* runNumber) {
	// load trees
	TString output_path = inputPath;
	output_path.ReplaceAll(".root", "_scaled.root");
	TFile* inputFile = TFile::Open(inputPath);
	TTree* inputTree = (TTree*)inputFile->Get("data");
	TFile* outputFile = new TFile(output_path, "RECREATE");
	outputFile->cd();
	TTree* outputTree = inputTree->CloneTree(0);
	cout << "Scaling events for " << inputPath << " with totMeans_" << runNumber << ".tsv" << endl;

	Float_t energy;
	Float_t tot, totShifted;
	UInt_t channelId; 
	Int_t xi;
	Int_t yi;
	inputTree->SetBranchAddress("energy", &energy);
	inputTree->SetBranchAddress("tot", &tot);
	inputTree->SetBranchAddress("channelID", &channelId);
	inputTree->SetBranchAddress("xi", &xi);
	inputTree->SetBranchAddress("yi", &yi);

	inputTree->SetBranchStatus("*", 0);
	inputTree->SetBranchStatus("time", 1);
	inputTree->SetBranchStatus("energy", 1);
	inputTree->SetBranchStatus("tot", 1);
	inputTree->SetBranchStatus("channelID", 1);
	inputTree->SetBranchStatus("xi", 1);
	inputTree->SetBranchStatus("yi", 1);

	outputTree->SetBranchAddress("tot", &totShifted);

	// load tot means from tsv
	const char* layers[3] = {"X","Y","U"};
	vector <vector <Float_t>> totMeans(3);
	for (Int_t i = 0; i < 3; i++) {
		ifstream tsv(Form("totMeans%s_%s.tsv", layers[i], runNumber));
		string line;

		// skip header
		getline(tsv, line);
		totMeans[i].push_back(0);

		while (getline(tsv, line)) {
			istringstream ss(line);
			Int_t xiBin;
			Float_t mean;
			ss >> xiBin >> mean;

			totMeans[i].push_back(mean);
		}
	}

	// scale tot in events
	Long64_t entries = inputTree->GetEntries();
	for (Long64_t entry = 0; entry < entries; entry++) {
		printProgress(entry, entries);

		inputTree->GetEntry(entry);
		if ((channelId == 4128 && energy ==5) || totMeans[yi][xi] == 0) {
			totShifted = tot;
		} else {
			totShifted = tot * TOT_TARGET / totMeans[yi][xi];
		}

		outputTree->Fill();
	}

	outputTree->Write();
	outputFile->Close();
	inputFile->Close();

	cout << endl << "Saved to: " << output_path << endl;
}

