#include <TFile.h>
#include <TSystem.h>
#include <TTree.h>
#include <TString.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "utils/createOutFile.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TFile* outFileScaled = nullptr;
TTree* outTreeScaled = nullptr;
Float_t totScaled;
vector<vector<Float_t>> totMeans(3);

void processScaleStart(const DataFBT1& inData, const char* runNumber) {
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	// create output file and tree
	outTreeScaled = inData.tree->CloneTree(0);
	outTreeScaled->SetBranchAddress("tot", &totScaled);

	// load tot means from tsv
	for (UInt_t i = 0; i < 3; i++) {
		TString totMeansFile = Form(
			"%s_totMeans%c.tsv", 
			runNumber, LAYER_NAMES[i]
		);
		ifstream tsv(totMeansFile.Data());
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
}

void processScaleLoop(const DataFBT1& inData) {
	// scale tot in events
	if (inData.channelId == 4128 || totMeans[inData.yi][inData.xi] == 0) {
		totScaled = inData.tot;
	} else {
		totScaled = inData.tot * TOT_SCALE_TARGET / totMeans[inData.yi][inData.xi];
	}
	outTreeScaled->Fill();
}

void processScaleEnd() {
	outTreeScaled->Write();
	outFileScaled->Close();
}

void processScale(const TString& inPath, const char* runNumber) {
	cout << "Scaling events for " << inPath << " with " << runNumber << "_totMeans.tsv" << endl;

	// load trees
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	outFileScaled = createOutFile(inPath, "_scaled.root");

	processScaleStart(inData, runNumber);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processScaleLoop(inData);
	}

	processScaleEnd();
}

