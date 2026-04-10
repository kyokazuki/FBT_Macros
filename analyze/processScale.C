#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "utils/parameters.C"
#include "utils/loadData.C"
#include "utils/createOutFile.C"
#include "utils/printProgress.C"

void processScale(const TString& inPath, const char* runNumber) {
	cout << "Scaling events for " << inPath << " with totMeans_" << runNumber << ".tsv" << endl;

	// load trees
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	// output tree
	TFile* outFile = createOutFile(inPath, "_scaled.root");
	TTree* outTree = inData.tree->CloneTree(0);

	Float_t totShifted;
	outTree->SetBranchAddress("tot", &totShifted);

	// load tot means from tsv
	vector <vector <Float_t>> totMeans(3);
	for (size_t i = 0; i < 3; i++) {
		ifstream tsv(Form("totMeans%s_%s.tsv", LAYERS[i], runNumber));
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
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);
		if (inData.channelId == 4128 || totMeans[inData.yi][inData.xi] == 0) {
			totShifted = inData.tot;
		} else {
			totShifted = inData.tot * TOT_SCALE_TARGET / totMeans[inData.yi][inData.xi];
		}

		outTree->Fill();
	}

	outTree->Write();
	outFile->Close();
}

