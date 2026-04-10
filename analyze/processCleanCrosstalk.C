#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>

#include "utils/parameters.C"
#include "utils/loadData.C"
#include "utils/createOutFile.C"
#include "utils/printProgress.C"

void processCleanCrosstalk(const TString& inPath) {
	cout << "Cleaning events for " << inPath << endl;

	const Double_t dtimeRange[2] = {0, 20e3};
	const Float_t totRatioRange[2] = {0, 0.2};
	const Int_t dxiRange[2] = {-3, 3};

	DataFBT2 inData({inPath}, "events");

	TFiles* outFile = createOutFile(inPath, "_cleanedC.root");
	TTree* outTree = inData.tree->CloneTree(0);
	vector<vector <Long64_t>*> 	timeVCleaned(3, nullptr);
	vector<vector <Float_t>*> 	energyVCleaned(3, nullptr);
	vector<vector <Float_t>*> 	totVCleaned(3, nullptr);
	vector<vector <UInt_t>*> 	channelIdVCleaned(3, nullptr);
	vector<vector <Int_t>*> 	xiVCleaned(3, nullptr);
	for (Int_t i = 0; i < 3; i++) {
		outTree->SetBranchAddress(Form("time%s", LAYERS[i]), &timeVCleaned[i]);
		outTree->SetBranchAddress(Form("energy%s", LAYERS[i]), &energyVCleaned[i]);
		outTree->SetBranchAddress(Form("tot%s", LAYERS[i]), &totVCleaned[i]);
		outTree->SetBranchAddress(Form("channelID%s", LAYERS[i]), &channelIdVCleaned[i]);
		outTree->SetBranchAddress(Form("xi%s", LAYERS[i]), &xiVCleaned[i]);
	}

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		// reset cleaned vectors
		for (Int_t layer = 0; layer < 3; layer++) {
			timeVCleaned[layer]->clear();
			energyVCleaned[layer]->clear();
			totVCleaned[layer]->clear();
			channelIdVCleaned[layer]->clear();
			xiVCleaned[layer]->clear();
		}

		for (Int_t layer = 0; layer < 3; layer++) {
			// fill directly if 0 or 1 entry
			if (inData.timeV[layer]->size() <= 1) {
				*timeVCleaned[layer] = *inData.timeV[layer];
				*energyVCleaned[layer] = *inData.energyV[layer];
				*totVCleaned[layer] = *inData.totV[layer];
				*channelIdVCleaned[layer] = *inData.channelIdV[layer];
				*xiVCleaned[layer] = *inData.xiV[layer];

				continue;
			}

			// fill biggest tot regardless
			timeVCleaned[layer]			->push_back((*inData.timeV[layer])[0]);
			energyVCleaned[layer]		->push_back((*inData.energyV[layer])[0]);
			totVCleaned[layer]			->push_back((*inData.totV[layer])[0]);
			channelIdVCleaned[layer]	->push_back((*inData.channelIdV[layer])[0]);
			xiVCleaned[layer]			->push_back((*inData.xiV[layer])[0]);

			// check for crosstalk
			for (UInt_t i = 1; i < inData.timeV[layer]->size(); i++) {
				Long64_t dtime = (*inData.timeV[layer])[i] - (*inData.timeV[layer])[0];
				Int_t dxi = (*inData.xiV[layer])[i] - (*inData.xiV[layer])[0];
				Float_t totRatio = (Float_t) (*inData.totV[layer])[i] / (Float_t) (*inData.totV[layer])[0];

				// fill if not crosstalk
				if (!(
					dtime > dtimeRange[0] && dtime < dtimeRange[1] && 
					totRatio > totRatioRange[0] && totRatio < totRatioRange[1] && 
					dxi >= dxiRange[0] && dxi <= dxiRange[1]
				)) {
					timeVCleaned[layer]			->push_back((*inData.timeV[layer])[i]);
					energyVCleaned[layer]		->push_back((*inData.energyV[layer])[i]);
					totVCleaned[layer]			->push_back((*inData.totV[layer])[i]);
					channelIdVCleaned[layer]	->push_back((*inData.channelIdV[layer])[i]);
					xiVCleaned[layer]			->push_back((*inData.xiV[layer])[i]);
				}
			}
		}
		outTree->Fill();
	}

	outFile->cd();
	outTree->Write();
	outFile->Close();
}

