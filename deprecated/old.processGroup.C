#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>

#include "utils/printProgress.C"

// const Long64_t DT_RANGE[2] 		= {-285000, -250000};	// deuterium
const Long64_t DT_RANGE[2] 		= {-295000, -270000};	// He3
// const Long64_t DT_RANGE[2] 		= {-3880000, -3840000};	// 2512 RARiS

void processGroup(const TString& inputPath) {
	TString output_path = inputPath;
	output_path.ReplaceAll(".root", "_grouped.root");
	TFile* inputFile = TFile::Open(inputPath);
	TFile* outputFile = new TFile(output_path, "RECREATE");
	TTree* inputTree = (TTree*)inputFile->Get("data");
	TTree* outputTree = new TTree("events", "outputTree");
	cout << "Grouping events for " << inputPath << endl;

	Long64_t time; 
	Float_t energy;
	Float_t tot;
	UInt_t channelId; 
	Int_t xi;
	Int_t yi;
	inputTree->SetBranchAddress("time", &time);
	inputTree->SetBranchAddress("energy", &energy);
	inputTree->SetBranchAddress("tot", &tot);
	inputTree->SetBranchAddress("channelID", &channelId);
	inputTree->SetBranchAddress("xi", &xi);
	inputTree->SetBranchAddress("yi", &yi);

	inputTree->SetBranchStatus("*", 0);
	inputTree->SetBranchStatus("time", 1);
	inputTree->SetBranchStatus("energy", 1);
	inputTree->SetBranchStatus("tot", 1);
	inputTree->SetBranchStatus("channelID", 1);
	inputTree->SetBranchStatus("xi", 1);
	inputTree->SetBranchStatus("yi", 1);

	vector<vector <Long64_t>> timeVectors(3);
	vector<vector <Long64_t>> timeVectorsSorted(3);
	vector<vector <Float_t>> energyVectors(3);
	vector<vector <Float_t>> energyVectorsSorted(3);
	vector<vector <Float_t>> totVectors(3);
	vector<vector <Float_t>> totVectorsSorted(3);
	vector<vector <UInt_t>> channelIdVectors(3);
	vector<vector <UInt_t>> channelIdVectorsSorted(3);
	vector<vector <Int_t>> xiVectors(3);
	vector<vector <Int_t>> xiVectorsSorted(3);
	vector<Long64_t> timeGate;
	const char* surfaces[3] = {"X","Y","U"};
	for (Int_t i = 0; i < 3; i++) {
		outputTree->Branch(Form("time%s", surfaces[i]), &timeVectorsSorted[i]);
		outputTree->Branch(Form("energy%s", surfaces[i]), &energyVectorsSorted[i]);
		outputTree->Branch(Form("tot%s", surfaces[i]), &totVectorsSorted[i]);
		outputTree->Branch(Form("channelID%s", surfaces[i]), &channelIdVectorsSorted[i]);
		outputTree->Branch(Form("xi%s", surfaces[i]), &xiVectorsSorted[i]);
	}
	outputTree->Branch("timeGate", &timeGate);

	Long64_t entries = inputTree->GetEntries();
	Long64_t gateRise;
	Long64_t dt;

	for (Long64_t entry = 0; entry < entries; entry++) {
		printProgress(entry, entries);

		inputTree->GetEntry(entry);
		if (!(channelId == 4128 && energy == 5)) {
			continue;
		}

		// append gate entries to timeGate
		gateRise = time;
		timeGate.push_back(gateRise);
		for (Long64_t entry2 = entry + 1; entry2 >= 0 && entry2 < entries; entry2++) {
			inputTree->GetEntry(entry2);
			if (!(channelId == 4128 && energy == -5)) {
				continue;
			} else {
				timeGate.push_back(time);
				break;
			}
		}

		// append fiber entries
		for (Int_t dir = -1; dir <= 1; dir += 2) {
			for (Long64_t entry2 = entry + dir; entry2 >= 0 && entry2 < entries; entry2 += dir) {
				inputTree->GetEntry(entry2);
				dt = time - gateRise;

				if ((dir == -1 && dt > DT_RANGE[1]) || (dir == 1 && dt < DT_RANGE[0])) {
					continue;
				} else if ((dir == -1 && dt < DT_RANGE[0]) || (dir == 1 && dt > DT_RANGE[1])) {
					break;
				}

				timeVectors[yi].push_back(time);
				energyVectors[yi].push_back(energy);
				totVectors[yi].push_back(tot);
				channelIdVectors[yi].push_back(channelId);
				xiVectors[yi].push_back(xi);
			}
		}

		vector<vector <size_t>> indices(3);
		for (Int_t i = 0; i < 3; i++) {
			for (size_t j = 0; j < totVectors[i].size(); j++) {
				indices[i].push_back(j);
			}

			vector<Float_t> tot_sort = totVectors[i];
			sort(indices[i].begin(), indices[i].end(), [&tot_sort](size_t i1, size_t i2) {
				return tot_sort[i1] > tot_sort[i2];
			});

			for (size_t j : indices[i]) {
				timeVectorsSorted[i].push_back(timeVectors[i][j]);
				energyVectorsSorted[i].push_back(energyVectors[i][j]);
				totVectorsSorted[i].push_back(totVectors[i][j]);
				channelIdVectorsSorted[i].push_back(channelIdVectors[i][j]);
				xiVectorsSorted[i].push_back(xiVectors[i][j]);
			}
		}

		outputTree->Fill();

		for (Int_t i = 0; i < 3; i++) {
			timeVectors[i].clear();
			timeVectorsSorted[i].clear();
			energyVectors[i].clear();
			energyVectorsSorted[i].clear();
			totVectors[i].clear();
			totVectorsSorted[i].clear();
			channelIdVectors[i].clear();
			channelIdVectorsSorted[i].clear();
			xiVectors[i].clear();
			xiVectorsSorted[i].clear();
			timeGate.clear();
		}
	}

	outputFile->cd();
	outputTree->Write();
	outputFile->Close();
	inputFile->Close();

	cout << endl << "Saved to: " << output_path << endl;
}

