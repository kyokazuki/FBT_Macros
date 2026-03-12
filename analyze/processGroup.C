#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"

// const Long64_t DT_RANGE[2] 		= {-285000, -250000};	// deuterium
// const Long64_t DT_RANGE[2] 		= {-295000, -270000};	// He3
const Long64_t DT_RANGE[2] 		= {-3880000, -3840000};	// 2512 RARiS

void processGroup(const TString& inPath) {
	cout << "Grouping events for " << inPath << endl;

	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	TString outPath = inPath;
	outPath.ReplaceAll(".root", "_grouped.root");
	TFile* outFile = new TFile(outPath, "RECREATE");
	outFile->cd();
	TTree* outTree = new TTree("events", "outTree");

	vector<vector <Long64_t>> timeV(3);
	vector<vector <Long64_t>> timeVSorted(3);
	vector<vector <Float_t>> energyV(3);
	vector<vector <Float_t>> energyVSorted(3);
	vector<vector <Float_t>> totV(3);
	vector<vector <Float_t>> totVSorted(3);
	vector<vector <UInt_t>> channelIdV(3);
	vector<vector <UInt_t>> channelIdVSorted(3);
	vector<vector <Int_t>> xiV(3);
	vector<vector <Int_t>> xiVSorted(3);
	vector<Long64_t> timeGate;
	const char* surfaces[3] = {"X","Y","U"};
	for (Int_t i = 0; i < 3; i++) {
		outTree->Branch(Form("time%s", surfaces[i]), &timeVSorted[i]);
		outTree->Branch(Form("energy%s", surfaces[i]), &energyVSorted[i]);
		outTree->Branch(Form("tot%s", surfaces[i]), &totVSorted[i]);
		outTree->Branch(Form("channelID%s", surfaces[i]), &channelIdVSorted[i]);
		outTree->Branch(Form("xi%s", surfaces[i]), &xiVSorted[i]);
	}
	outTree->Branch("timeGate", &timeGate);

	Long64_t gateRise;
	Long64_t dt;

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);
		if (!(inData.channelId == 4128 && inData.energy == 5)) {
			continue;
		}

		// append gate inData.entries to timeGate
		gateRise = inData.time;
		timeGate.push_back(gateRise);
		for (Long64_t entry2 = entry + 1; entry2 >= 0 && entry2 < inData.entries; entry2++) {
			inData.tree->GetEntry(entry2);
			if (!(inData.channelId == 4128 && inData.energy == -5)) {
				continue;
			} else {
				timeGate.push_back(inData.time);
				break;
			}
		}

		// append fiber entries
		for (Int_t dir = -1; dir <= 1; dir += 2) {
			for (Long64_t entry2 = entry + dir; entry2 >= 0 && entry2 < inData.entries; entry2 += dir) {
				inData.tree->GetEntry(entry2);
				dt = inData.time - gateRise;

				if ((dir == -1 && dt > DT_RANGE[1]) || (dir == 1 && dt < DT_RANGE[0])) {
					continue;
				} else if ((dir == -1 && dt < DT_RANGE[0]) || (dir == 1 && dt > DT_RANGE[1])) {
					break;
				}

				timeV[inData.yi].push_back(inData.time);
				energyV[inData.yi].push_back(inData.energy);
				totV[inData.yi].push_back(inData.tot);
				channelIdV[inData.yi].push_back(inData.channelId);
				xiV[inData.yi].push_back(inData.xi);
			}
		}

		vector<vector <size_t>> indices(3);
		for (Int_t i = 0; i < 3; i++) {
			for (size_t j = 0; j < totV[i].size(); j++) {
				indices[i].push_back(j);
			}

			vector<Float_t> tot_sort = totV[i];
			sort(indices[i].begin(), indices[i].end(), [&tot_sort](size_t i1, size_t i2) {
				return tot_sort[i1] > tot_sort[i2];
			});

			for (size_t j : indices[i]) {
				timeVSorted[i].push_back(timeV[i][j]);
				energyVSorted[i].push_back(energyV[i][j]);
				totVSorted[i].push_back(totV[i][j]);
				channelIdVSorted[i].push_back(channelIdV[i][j]);
				xiVSorted[i].push_back(xiV[i][j]);
			}
		}

		outTree->Fill();

		for (Int_t i = 0; i < 3; i++) {
			timeV[i].clear();
			timeVSorted[i].clear();
			energyV[i].clear();
			energyVSorted[i].clear();
			totV[i].clear();
			totVSorted[i].clear();
			channelIdV[i].clear();
			channelIdVSorted[i].clear();
			xiV[i].clear();
			xiVSorted[i].clear();
			timeGate.clear();
		}
	}

	outTree->Write();
	outFile->Close();

	cout << endl << "Saved to: " << outPath << endl;
}

