#include <TFile.h>
#include <TSystem.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>

#include <iostream>
#include <stdlib.h>
#include <vector>

#include "utils/createOutFile.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TFile* outFileSelected = nullptr;
TTree* outTreeSelected = nullptr;
vector<vector<Float_t>*> totSelectedV{3, nullptr};
vector<vector<Float_t>*> energySelectedV{3, nullptr};
vector<vector<Long64_t>*> timeSelectedV{3, nullptr};
vector<vector<Int_t>*> xiSelectedV{3, nullptr};
vector<vector<UInt_t>*> channelIdSelectedV{3, nullptr};
Int_t xiWindow[3][2] = {{80, 230}, {43, 193}, {1, 320}};

void processSelectStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("*", 1);

	// create output file and tree
	outTreeSelected = inData.tree->CloneTree(0);

	for (Int_t layer = 0; layer < 3; layer++) {
		outTreeSelected->SetBranchAddress(Form("tot%c", LAYER_NAMES[layer]), &totSelectedV[layer]);
		outTreeSelected->SetBranchAddress(Form("energy%c", LAYER_NAMES[layer]), &energySelectedV[layer]);
		outTreeSelected->SetBranchAddress(Form("time%c", LAYER_NAMES[layer]), &timeSelectedV[layer]);
		outTreeSelected->SetBranchAddress(Form("xi%c", LAYER_NAMES[layer]), &xiSelectedV[layer]);
		outTreeSelected->SetBranchAddress(Form("channelID%c", LAYER_NAMES[layer]), &channelIdSelectedV[layer]);
	}
}

void processSelectLoop(const DataFBT2& inData) {
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.timeV[layer]->size(); hit++) {
			if (inRange((*inData.xiV[layer])[hit], xiWindow[layer])) {
				(*totSelectedV[layer]).push_back((*inData.totV[layer])[hit]);
				(*energySelectedV[layer]).push_back((*inData.energyV[layer])[hit]);
				(*timeSelectedV[layer]).push_back((*inData.timeV[layer])[hit]);
				(*xiSelectedV[layer]).push_back((*inData.xiV[layer])[hit]);
				(*channelIdSelectedV[layer]).push_back((*inData.channelIdV[layer])[hit]);
			}
		}
	}

	outTreeSelected->Fill();

	for (Int_t layer = 0; layer < 3; layer++) {
		(*totSelectedV[layer]).clear();
		(*energySelectedV[layer]).clear();
		(*timeSelectedV[layer]).clear();
		(*xiSelectedV[layer]).clear();
		(*channelIdSelectedV[layer]).clear();
	}
}

void processSelectEnd() {
	outTreeSelected->Write();
	outFileSelected->Close();
}

void processSelect(const TString& inPath) {
	cout << "Selecting events for " << inPath << endl;

	// load trees
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	outFileSelected = createOutFile(inPath, "_selected.root");

	processSelectStart(inData);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processSelectLoop(inData);
	}

	processSelectEnd();
}

