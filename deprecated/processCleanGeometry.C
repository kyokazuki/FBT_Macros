#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>

#include "utils/printProgress.C"

void processCleanGeometry(const TString& inputPath) {
	TString output_path = inputPath;
	output_path.ReplaceAll(".root", "_cleanedG.root");
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
	const char* surfaces[3] = {"X","Y","U"};
	for (Int_t i = 0; i < 3; i++) {
		inputTree->SetBranchAddress(Form("time%s", surfaces[i]), &timeVectors[i]);
		inputTree->SetBranchAddress(Form("energy%s", surfaces[i]), &energyVectors[i]);
		inputTree->SetBranchAddress(Form("tot%s", surfaces[i]), &totVectors[i]);
		inputTree->SetBranchAddress(Form("channelID%s", surfaces[i]), &channelIdVectors[i]);
		inputTree->SetBranchAddress(Form("xi%s", surfaces[i]), &xiVectors[i]);

		outputTree->SetBranchAddress(Form("time%s", surfaces[i]), &timeVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("energy%s", surfaces[i]), &energyVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("tot%s", surfaces[i]), &totVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("channelID%s", surfaces[i]), &channelIdVectorsCleaned[i]);
		outputTree->SetBranchAddress(Form("xi%s", surfaces[i]), &xiVectorsCleaned[i]);
	}

	Long64_t events = inputTree->GetEntries();
	struct totEntry {
		Float_t tot;
		Int_t surface;
	};
	vector<totEntry> totEntries;

	for (Long64_t event = 0; event < events; event++) {
		printProgress(event, events);

		inputTree->GetEntry(event);

		// fill directly if not enough entries in each surface
		if (xiVectors[0]->size() == 0 || xiVectors[1]->size() == 0 || xiVectors[2]->size() == 0) {
			timeVectorsCleaned = timeVectors;
			energyVectorsCleaned = energyVectors;
			totVectorsCleaned = totVectors;
			channelIdVectorsCleaned = channelIdVectors;
			xiVectorsCleaned = xiVectors;

			outputTree->Fill();
			continue;
		}

		// clear the vectors
		totEntries.clear();
		for (Int_t i = 0; i < 3; i++) {
			timeVectorsCleaned[i]->clear();
			energyVectorsCleaned[i]->clear();
			totVectorsCleaned[i]->clear();
			channelIdVectorsCleaned[i]->clear();
			xiVectorsCleaned[i]->clear();
		}

		// sort for the biggest 2 tots 
		for (Int_t i = 0; i < 3; i++) {
			totEntries.push_back({(*totVectors[i])[0], i});
		}
		sort(totEntries.begin(), totEntries.end(),
			 [](const totEntry& a, const totEntry& b) {
				 return a.tot > b.tot;
			 });

		// append the 2 entries
		for (Int_t i = 0; i < 2; i++) {
			Int_t s = totEntries[i].surface;
			timeVectorsCleaned[s]->push_back((*timeVectors[s])[0]);
			energyVectorsCleaned[s]->push_back((*energyVectors[s])[0]);
			totVectorsCleaned[s]->push_back((*totVectors[s])[0]);
			channelIdVectorsCleaned[s]->push_back((*channelIdVectors[s])[0]);
			xiVectorsCleaned[s]->push_back((*xiVectors[s])[0]);
		}

		// look for the last entry
		Int_t lastXi;
		if (totEntries[2].surface == 0) {
			lastXi = ((*xiVectors[2])[0] + 39.14285) / 0.73142 - (*xiVectors[1])[0];
		} else if (totEntries[2].surface == 1) {
			lastXi = ((*xiVectors[2])[0] + 39.14285) / 0.73142 - (*xiVectors[0])[0];
		} else {
			lastXi = 0.73142 * ((*xiVectors[0])[0] + (*xiVectors[1])[0]) - 39.14285;
		}
		Int_t s = totEntries[2].surface;
		for (UInt_t i = 0; i < totVectors[s]->size(); i++) {
			if ((*xiVectors[s])[i] >= lastXi - 2 && (*xiVectors[s])[i] <= lastXi + 2) {
				// append the last entry
				timeVectorsCleaned[s]->push_back((*timeVectors[s])[i]);
				energyVectorsCleaned[s]->push_back((*energyVectors[s])[i]);
				totVectorsCleaned[s]->push_back((*totVectors[s])[i]);
				channelIdVectorsCleaned[s]->push_back((*channelIdVectors[s])[i]);
				xiVectorsCleaned[s]->push_back((*xiVectors[s])[i]);
				break;
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

