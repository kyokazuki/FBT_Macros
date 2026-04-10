#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <iomanip>

#include "utils/parameters.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

void getBeamProfile(const TString& inPath) {
	DataFBT1 inData({inPath}, "data");

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	cout << "Getting beam profile from " << inPath << endl;

	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	vector <vector <Long64_t>> fiberEntries(3);
	for (Int_t layer = 0; layer < 3; layer++) {
		fiberEntries[layer].assign(LAYER_CHANNELS[layer] + 1, 0);
	}

	// count entries
	cout << "Counting entries..." << endl;
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);
		if (inData.xi == 0) continue;
		fiberEntries[inData.yi][inData.xi]++;
	}

	// print to tsv
	cout << "Printing to tsv..." << endl;
	Long64_t totalFiberEntries = inData.tree->GetEntries("xi != 0");
	for (Int_t layer = 0; layer < 3; layer++) {
		ofstream out(Form("beamProfile%s_%s.tsv", LAYERS[layer], runNumber.Data()));
		out << "xi\tcontribution" << endl;
		out << fixed << setprecision(8);
		for (Int_t channel = 1; channel <= LAYER_CHANNELS[layer]; channel++) {
			Float_t contribution = (Float_t) fiberEntries[layer][channel] / (Float_t) totalFiberEntries;
			out << channel << "\t" << contribution << endl;
		}
	}
}
