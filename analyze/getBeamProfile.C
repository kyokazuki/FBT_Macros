#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <iomanip>

const char* layers[3] = {"X", "Y", "U"};
const Int_t layerChannels[3] = {320, 224, 320};

void getBeamProfile(const TString& inputPath) {
	TFile* inputFile = TFile::Open(inputPath, "READ");
	TTree* inputTree = (TTree*)inputFile->Get("data");
	TString runNumber = TString(gSystem->BaseName(inputPath))(0,4);
	cout << "Getting beam profile from " << inputPath << endl;

	Int_t xi, yi;
	inputTree->SetBranchStatus("*", 0);
	inputTree->SetBranchStatus("xi", 1);
	inputTree->SetBranchStatus("yi", 1);
	inputTree->SetBranchAddress("xi", &xi);
	inputTree->SetBranchAddress("yi", &yi);

	vector <vector <Long64_t>> fiberEntries(3);
	for (Int_t layer = 0; layer < 3; layer++) {
		fiberEntries[layer].assign(layerChannels[layer] + 1, 0);
	}

	// count entries
	cout << "Counting entries..." << endl;
	Long64_t entries = inputTree->GetEntries();
	for (Int_t entry = 0; entry < entries; entry++) {
		if (entry % 10000 == 0 || entry == entries - 1) {
			cout << "\rEntry: " << entry + 1 << "/" << entries << flush;
		}
		inputTree->GetEntry(entry);
		if (xi == 0) continue;
		fiberEntries[yi][xi]++;
	}
	cout << endl;

	// print to tsv
	cout << "Printing to tsv..." << endl;
	Long64_t totalFiberEntries = inputTree->GetEntries("xi != 0");
	for (Int_t layer = 0; layer < 3; layer++) {
		ofstream out(Form("beamProfile%s_%s.tsv", layers[layer], runNumber.Data()));
		out << "xi\tcontribution" << endl;
		out << fixed << setprecision(8);
		for (Int_t channel = 1; channel <= layerChannels[layer]; channel++) {
			Float_t contribution = (Float_t) fiberEntries[layer][channel] / (Float_t) totalFiberEntries;
			out << channel << "\t" << contribution << endl;
		}
	}
	inputFile->Close();
}
