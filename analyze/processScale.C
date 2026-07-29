#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include <TFile.h>
#include <TSystem.h>
#include <TTree.h>
#include <TString.h>
#include <TMath.h>

#include "utils/createOutFile.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TFile* outFileScaled = nullptr;
TTree* outTreeScaled = nullptr;
vector<vector<Float_t>*> totScaledV{3, nullptr};
vector<vector<Long64_t>*> timeCorrectedV{3, nullptr};
vector<vector<Float_t>> totMeans(3);

void processScaleStart(const DataFBT2& inData, const char* inName) {
	inData.tree->SetBranchStatus("*", 1);

	// create output file and tree
	outTreeScaled = inData.tree->CloneTree(0);

	for (Int_t layer = 0; layer < 3; layer++) {
		outTreeScaled->SetBranchAddress(Form("tot%c", LAYER_NAMES[layer]), &totScaledV[layer]);
		outTreeScaled->SetBranchAddress(Form("time%c", LAYER_NAMES[layer]), &timeCorrectedV[layer]);
	}

	// load tot means from tsv
	for (Int_t layer = 0; layer < 3; layer++) {
		ifstream tsv(Form(
			"%s%c.tsv", 
			inName, LAYER_NAMES[layer]));
		string line;

		// skip header
		getline(tsv, line);
		totMeans[layer].push_back(0);

		while (getline(tsv, line)) {
			istringstream ss(line);
			Int_t xiBin;
			Float_t mean;
			ss >> xiBin >> mean;

			totMeans[layer].push_back(mean);
		}
	}
}

void processScaleLoop(const DataFBT2& inData) {
	// scale tot in events
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			Long64_t slewOffset = (Long64_t) (SLEW_A * TMath::Exp(SLEW_B * (*inData.totV[layer])[hit]) + SLEW_C);
			Long64_t timeCorrected = (*inData.timeV[layer])[hit] - slewOffset;
			(*timeCorrectedV[layer]).push_back(timeCorrected);

			Float_t totScaled = (*inData.totV[layer])[hit] * TOT_SCALE_TARGET / totMeans[layer][(*inData.xiV[layer])[hit]];
			(*totScaledV[layer]).push_back(totScaled);
		}
	}

	outTreeScaled->Fill();

	for (Int_t layer = 0; layer < 3; layer++) {
		(*timeCorrectedV[layer]).clear();
		(*totScaledV[layer]).clear();
	}
}

void processScaleEnd() {
	outTreeScaled->Write();
	outFileScaled->Close();
}

// inName: "1074_totMeans" or "1074_totMeansCoined"
void processScale(const TString& inPath, const char* inName) {
	cout << "Scaling events for " << inPath << " with " << inName << "[X-U].tsv" << endl;

	// load trees
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	outFileScaled = createOutFile(inPath, "_scaled.root");

	processScaleStart(inData, inName);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processScaleLoop(inData);
	}

	processScaleEnd();
}

