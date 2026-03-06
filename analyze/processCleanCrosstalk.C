#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>

#include "utils/printProgress.C"

const Double_t dtimeRange[2] = {0, 20e3};
const Float_t totRatioRange[2] = {0, 0.2};
const Int_t dxiRange[2] = {-3, 3};

void processCleanCrosstalk(const TString& inputPath) {
	TString output_path = inputPath;
	output_path.ReplaceAll(".root", "_cleanedC.root");
	TFile* inputFile = TFile::Open(inputPath);
	TTree* inputTree = (TTree*)inputFile->Get("events");
	TFile* outputFile = new TFile(output_path, "RECREATE");
	TTree* outputTree = inputTree->CloneTree(0);
	outputFile->cd();
	cout << "Cleaning events for " << inputPath << endl;

	vector<vector <Long64_t>*> 	timeVectors(3, nullptr);
	vector<vector <Long64_t>*> 	timeVectorsCleaned(3, nullptr);
	vector<vector <Float_t>*> 	energyVectors(3, nullptr);
	vector<vector <Float_t>*> 	energyVectorsCleaned(3, nullptr);
	vector<vector <Float_t>*> 	totVectors(3, nullptr);
	vector<vector <Float_t>*> 	totVectorsCleaned(3, nullptr);
	vector<vector <UInt_t>*> 	channelIdVectors(3, nullptr);
	vector<vector <UInt_t>*> 	channelIdVectorsCleaned(3, nullptr);
	vector<vector <Int_t>*> 	xiVectors(3, nullptr);
	vector<vector <Int_t>*> 	xiVectorsCleaned(3, nullptr);
	const char* layers[3] = {"X","Y","U"};
	for (Int_t i = 0; i < 3; i++) {
		inputTree->SetBranchAddress(Form("time%s", layers[i]), &timeVectors[i]);
		inputTree->SetBranchAddress(Form("energy%s", layers[i]), &energyVectors[i]);
		inputTree->SetBranchAddress(Form("tot%s", layers[i]), &totVectors[i]);
		inputTree->SetBranchAddress(Form("channelID%s", layers[i]), &channelIdVectors[i]);
		inputTree->SetBranchAddress(Form("xi%s", layers[i]), &xiVectors[i]);

		outputTree->SetBranchAddress(Form("time%s", layers[i]), &timeVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("energy%s", layers[i]), &energyVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("tot%s", layers[i]), &totVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("channelID%s", layers[i]), &channelIdVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("xi%s", layers[i]), &xiVectorsCleaned[i]);
	}

	Long64_t events = inputTree->GetEntries();

	for (Long64_t event = 0; event < events; event++) {
		printProgress(event, events);

		inputTree->GetEntry(event);

		// reset cleaned vectors
		for (Int_t layer = 0; layer < 3; layer++) {
			timeVectorsCleaned[layer]->clear();
			energyVectorsCleaned[layer]->clear();
			totVectorsCleaned[layer]->clear();
			channelIdVectorsCleaned[layer]->clear();
			xiVectorsCleaned[layer]->clear();
		}

		for (Int_t layer = 0; layer < 3; layer++) {
			// fill directly if 0 or 1 entry
			if (timeVectors[layer]->size() <= 1) {
				*timeVectorsCleaned[layer] = *timeVectors[layer];
				*energyVectorsCleaned[layer] = *energyVectors[layer];
				*totVectorsCleaned[layer] = *totVectors[layer];
				*channelIdVectorsCleaned[layer] = *channelIdVectors[layer];
				*xiVectorsCleaned[layer] = *xiVectors[layer];

				continue;
			}

			// fill biggest tot regardless
			timeVectorsCleaned[layer]->push_back((*timeVectors[layer])[0]);
			energyVectorsCleaned[layer]->push_back((*energyVectors[layer])[0]);
			totVectorsCleaned[layer]->push_back((*totVectors[layer])[0]);
			channelIdVectorsCleaned[layer]->push_back((*channelIdVectors[layer])[0]);
			xiVectorsCleaned[layer]->push_back((*xiVectors[layer])[0]);

			// check for crosstalk
			for (UInt_t i = 1; i < timeVectors[layer]->size(); i++) {
				Long64_t dtime = (*timeVectors[layer])[i] - (*timeVectors[layer])[0];
				Int_t dxi = (*xiVectors[layer])[i] - (*xiVectors[layer])[0];
				Float_t totRatio = (Float_t) (*totVectors[layer])[i] / (Float_t) (*totVectors[layer])[0];

				// fill if not crosstalk
				if (!(
					dtime > dtimeRange[0] && dtime < dtimeRange[1] && 
					totRatio > totRatioRange[0] && totRatio < totRatioRange[1] && 
					dxi >= dxiRange[0] && dxi <= dxiRange[1]
				)) {
					timeVectorsCleaned[layer]->push_back((*timeVectors[layer])[i]);
					energyVectorsCleaned[layer]->push_back((*energyVectors[layer])[i]);
					totVectorsCleaned[layer]->push_back((*totVectors[layer])[i]);
					channelIdVectorsCleaned[layer]->push_back((*channelIdVectors[layer])[i]);
					xiVectorsCleaned[layer]->push_back((*xiVectors[layer])[i]);
				}
			}
		}
		outputTree->Fill();
	}

	outputFile->cd();
	outputTree->Write();
	outputFile->Close();
	inputFile->Close();

	cout << "Saved to: " << output_path << endl;
}

