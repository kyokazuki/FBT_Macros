#include <iostream>
#include <stdlib.h>
#include <vector>

#include <TFile.h>
#include <TSystem.h>
#include <TTree.h>
#include <TString.h>
#include <TMath.h>

#include "utils/createOutFile.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TFile* outFileSlewed = nullptr;
TTree* outTreeSlewed = nullptr;
vector<vector<Long64_t>*> timeSlewedV{3, nullptr};

void processSlewStart(const DataFBT2& inData) {
	inData.tree->SetBranchStatus("*", 1);

	// create output file and tree
	outTreeSlewed = inData.tree->CloneTree(0);

	for (Int_t layer = 0; layer < 3; layer++) {
		outTreeSlewed->SetBranchAddress(Form("time%c", LAYER_NAMES[layer]), &timeSlewedV[layer]);
	}
}

void processSlewLoop(const DataFBT2& inData) {
	// scale tot in events
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			// use different parameters for each layer
			Long64_t slewOffset = 
				(Long64_t) (SLEW_A[layer] * TMath::Exp(SLEW_B[layer] * (*inData.totV[layer])[hit]) + SLEW_C[layer]);
			// use X layer parameters for all layers
			// Long64_t slewOffset = 
			// 	(Long64_t) (SLEW_A[0] * TMath::Exp(SLEW_B[0] * (*inData.totV[0])[hit]) + SLEW_C[0]);
			Long64_t timeSlewed = (*inData.timeV[layer])[hit] - slewOffset;
			(*timeSlewedV[layer]).push_back(timeSlewed);
		}
	}

	outTreeSlewed->Fill();

	for (Int_t layer = 0; layer < 3; layer++) {
		(*timeSlewedV[layer]).clear();
	}
}

void processSlewEnd() {
	outTreeSlewed->Write();
	outFileSlewed->Close();
}

// inName: "1074_totMeans" or "1074_totMeansCoined"
void processSlew(const TString& inPath) {
	cout << "Scaling events for " << inPath << endl;

	// load trees
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	outFileSlewed = createOutFile(inPath, "_slewed2.root");
	// outFileSlewed = createOutFile(inPath, "_slewed.root");

	processSlewStart(inData);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processSlewLoop(inData);
	}

	processSlewEnd();
}

