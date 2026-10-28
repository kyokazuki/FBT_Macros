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

TFile* outFileGated = nullptr;
TTree* outTreeGated = nullptr;
vector<vector<Long64_t>*> timeGatedV{3, nullptr};

void processGateStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("*", 1);

	// create output file and tree
	outTreeGated = inData.tree->CloneTree(0);

	for (Int_t layer = 0; layer < 3; layer++) {
		outTreeGated->SetBranchAddress(Form("time%c", LAYER_NAMES[layer]), &timeGatedV[layer]);
	}
}

void processGateLoop(const DataFBT2& inData) {
	// calculate gate offset once per event
	// 1250: 0 offsets; 2500: 1 offset; 3750: 2 offsets; 0 (5000): 3 offsets
	Int_t gateRemain = (*inData.timeGate)[0] % 5000;
	Int_t gateType = (gateRemain / 1250 + 3) % 4;
	Long64_t gateOffset = gateType * 2500;

	// Long64_t gateNum = (*inData.timeGate)[0] % 5000;
	// Long64_t gateOffset;
	// if (gateNum == 0) {
	// 	gateOffset = (4 - 1) * 2500;
	// } else {
	// 	gateOffset = (gateNum / 1250 - 1) * 2500;
	// }

	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.timeV[layer]->size(); hit++) {
			Long64_t timeGated = (*inData.timeV[layer])[hit] + gateOffset;
			(*timeGatedV[layer]).push_back(timeGated);
		}
	}

	outTreeGated->Fill();

	for (Int_t layer = 0; layer < 3; layer++) {
		(*timeGatedV[layer]).clear();
	}
}

void processGateEnd() {
	outTreeGated->Write();
	outFileGated->Close();
}

void processGate(const TString& inPath) {
	cout << "Gating timestamps for " << inPath << endl;

	// load trees
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	outFileGated = createOutFile(inPath, "_gated.root");

	processGateStart(inData);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processGateLoop(inData);
	}

	processGateEnd();
}

