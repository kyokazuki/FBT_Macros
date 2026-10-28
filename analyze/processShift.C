#include <TFile.h>
#include <TSystem.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "utils/createOutFile.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TFile* outFileShifted = nullptr;
TTree* outTreeShifted = nullptr;
vector<vector<Long64_t>> timeOffsets(3);

vector<vector<Long64_t>> timeVShifted(3);
vector<vector<Float_t>> energyVShifted(3);
vector<vector<Float_t>> totVShifted(3);
vector<vector<UInt_t>> channelIdVShifted(3);
vector<vector<Int_t>> xiVShifted(3);
vector<Long64_t> timeGateShifted;

void processShiftStart(DataFBTCoin& inData, const char* inName) {
	// branches settings
	inData.tree->SetBranchStatus("*", 1);

	for (Int_t layer = 0; layer < 3; layer++) {
		outTreeShifted->Branch(Form("time%c", LAYER_NAMES[layer]), &timeVShifted[layer]);
		outTreeShifted->Branch(Form("energy%c", LAYER_NAMES[layer]), &energyVShifted[layer]);
		outTreeShifted->Branch(Form("tot%c", LAYER_NAMES[layer]), &totVShifted[layer]);
		outTreeShifted->Branch(Form("channelID%c", LAYER_NAMES[layer]), &channelIdVShifted[layer]);
		outTreeShifted->Branch(Form("xi%c", LAYER_NAMES[layer]), &xiVShifted[layer]);
	}
	outTreeShifted->Branch("timeGate", &(*inData.timeGate));

	// load time offsets from tsv
	for (Int_t layer = 0; layer < 3; layer++) {
		ifstream tsv(Form(
			"%s%c.tsv", 
			inName, LAYER_NAMES[layer]
		));
		if (!tsv.is_open()) {
			cout << "Could not open " << inName << LAYER_NAMES[layer] << ".tsv" << endl;
			gSystem->Exit(1);
		}
		string line;

		// skip header
		getline(tsv, line);
		timeOffsets[layer].push_back(0);

		while (getline(tsv, line)) {
			istringstream ss(line);
			Int_t xiBin;
			Long64_t offset;
			ss >> xiBin >> offset;

			timeOffsets[layer].push_back(offset);
		}
	}
}

void processShiftLoop(
	const DataFBTCoin& inData,
	const Long64_t (&timingRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.timeV[layer]->size(); hit++) {
			Long64_t timeShifted =
				(*inData.timeV[layer])[hit]
				+ timeOffsets[layer][(*inData.xiV[layer])[hit]];

			if (!inRange(timeShifted - (*inData.timeGate)[0], timingRange)) {
				continue;
			}

			timeVShifted[layer].push_back(timeShifted);
			energyVShifted[layer].push_back((*inData.energyV[layer])[hit]);
			totVShifted[layer].push_back((*inData.totV[layer])[hit]);
			channelIdVShifted[layer].push_back((*inData.channelIdV[layer])[hit]);
			xiVShifted[layer].push_back((*inData.xiV[layer])[hit]);
		}
	}

	outTreeShifted->Fill();

	for (Int_t layer = 0; layer < 3; layer++) {
		timeVShifted[layer].clear();
		energyVShifted[layer].clear();
		totVShifted[layer].clear();
		channelIdVShifted[layer].clear();
		xiVShifted[layer].clear();
	}
}

void processShiftEnd() {
	outTreeShifted->Write();
	outFileShifted->Close();
}

void processShift(
	const TString& inPath, 
	const char* inName,
	const Long64_t (&timingRange)[2] = {-14000, 34000}
) {
	cout << "Shifting timestamps for " << inPath << " with " << inName << "[X-U].tsv" << endl;

	// load trees
	DataFBTCoin inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	outFileShifted = createOutFile(inPath, "_shifted.root");
	// outTreeShifted = inData.tree->CloneTree(0);
	outTreeShifted = new TTree("events", "outTree");

	processShiftStart(inData, inName);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processShiftLoop(inData, timingRange);
	}

	processShiftEnd();
}

