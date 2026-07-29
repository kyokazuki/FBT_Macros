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
vector<vector<Long64_t>*> timeShiftedV{3, nullptr};
vector<vector<Long64_t>> timeOffsets(3);

void processShiftStart(const DataFBT2& inData, const char* inName) {
	inData.tree->SetBranchStatus("*", 1);

	// create output file and tree
	outTreeShifted = inData.tree->CloneTree(0);

	for (Int_t layer = 0; layer < 3; layer++) {
		outTreeShifted->SetBranchAddress(Form("time%c", LAYER_NAMES[layer]), &timeShiftedV[layer]);
	}

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

void processShiftLoop(const DataFBT2& inData) {
	// calculate gate offset once per event
	// 1250: 0 offsets; 2500: 1 offset; 3750: 2 offsets; 5000 (0): 3 offsets
	Long64_t gateOffset;
	Long64_t gateNum = (*inData.timeGate)[0] % 5000;
	if (gateNum == 0) {
		gateOffset = (4 - 1) * 2500;
	} else {
		gateOffset = (gateNum / 1250 - 1) * 2500;
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.timeV[layer]->size(); hit++) {
			Long64_t timeShifted =
				(*inData.timeV[layer])[hit]
				+ timeOffsets[layer][(*inData.xiV[layer])[hit]]
				+ gateOffset;
			(*timeShiftedV[layer]).push_back(timeShifted);
		}
	}

	outTreeShifted->Fill();

	for (Int_t layer = 0; layer < 3; layer++) {
		(*timeShiftedV[layer]).clear();
	}
}

void processShiftEnd() {
	outTreeShifted->Write();
	outFileShifted->Close();
}

// inName: "1074_timeOffsets" or "1074_timeOffsetsCoined"
void processShift(const TString& inPath, const char* inName) {
	cout << "Shifting timestamps for " << inPath << " with " << inName << "[X-U].tsv" << endl;

	// load trees
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	outFileShifted = createOutFile(inPath, "_shifted.root");

	processShiftStart(inData, inName);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processShiftLoop(inData);
	}

	processShiftEnd();
}

