#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <algorithm>
#include <stdlib.h>

#include <unistd.h>

void treeGroup(const TString& inputPath) {
	// 2025/12 RARiS
	const Long64_t DT_RANGE[2] = {-3880000, -3840000};
	// const Long64_t DT_RANGE[2] = {-2380000, -2350000};
	// Deuterium
	// const Long64_t DT_RANGE[2] = {-285000, -250000};
	// He3
	// const Long64_t DT_RANGE[2] = {-295000, -270000};

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

	vector<vector <Long64_t>> timeVectors(3);
	vector<vector <Long64_t>> timeVectorsSorted(3);
	vector<vector <Long64_t>> energyVectors(3);
	vector<vector <Long64_t>> energyVectorsSorted(3);
	vector<vector <Long64_t>> totVectors(3);
	vector<vector <Long64_t>> totVectorsSorted(3);
	vector<vector <Long64_t>> channelIdVectors(3);
	vector<vector <Long64_t>> channelIdVectorsSorted(3);
	vector<vector <Long64_t>> xiVectors(3);
	vector<vector <Long64_t>> xiVectorsSorted(3);
	vector<string> surface = {"X", "Y", "U"};
	for (Int_t i = 0; i < 3; i++) {
		outputTree->Branch(Form("time%s", surface[i].c_str()), &timeVectorsSorted[i]);
		outputTree->Branch(Form("energy%s", surface[i].c_str()), &energyVectorsSorted[i]);
		outputTree->Branch(Form("tot%s", surface[i].c_str()), &totVectorsSorted[i]);
		outputTree->Branch(Form("channelID%s", surface[i].c_str()), &channelIdVectorsSorted[i]);
		outputTree->Branch(Form("xi%s", surface[i].c_str()), &xiVectorsSorted[i]);
	}

	Long64_t entries = inputTree->GetEntries();
	Long64_t time_tgr;
	Long64_t dt;
	Long64_t gate_count = 0;

	for (Long64_t entry = 0; entry < entries; entry++) {
		if (entry % 10000 == 0 || entry == entries - 1) {
			cout << "\rEntry: " << entry << "/" << entries << flush;
		}

		inputTree->GetEntry(entry);
		if (!(xi == 0 && energy == 5)) {
			continue;
		}

		gate_count += 1;
		time_tgr = time;

		// group trigger entry as well
		for (Int_t y = 0; y < 3; y++) {
			timeVectors[y].push_back(time);
			energyVectors[y].push_back(energy); //=5
			totVectors[y].push_back(tot); //=0
			channelIdVectors[y].push_back(channelId); //=4128
			xiVectors[y].push_back(xi); //=0
		}

		for (Int_t i = -1; i <= 1; i += 2) {
			for (Long64_t j = entry + i; j >= 0 && j < entries; j += i) {
				inputTree->GetEntry(j);
				dt = time - time_tgr;

				if ((xi == 0 && energy == 5) || (xi == 0 && energy == -5)) {
					break;
				} else if ((i == -1 && dt > DT_RANGE[1]) || (i == 1 && dt < DT_RANGE[0])) {
					continue;
				} else if ((i == -1 && dt < DT_RANGE[0]) || (i == 1 && dt > DT_RANGE[1])) {
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

			vector<Long64_t> tot_sort = totVectors[i];
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
		}
	}

	outputFile->cd();
	outputTree->Write();
	outputFile->Close();
	inputFile->Close();

	cout << endl << "Saved to: " << output_path << endl;
}

