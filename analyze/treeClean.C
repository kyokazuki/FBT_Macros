#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <algorithm>
#include <stdlib.h>

#include <unistd.h>

void treeClean(const TString& inputPath) {
	TString output_path = inputPath;
	output_path.ReplaceAll(".root", "_cleaned.root");
	TFile* inputFile = TFile::Open(inputPath);
	TTree* inputTree = (TTree*)inputFile->Get("events");
	TFile* outputFile = new TFile(output_path, "RECREATE");
	TTree* outputTree = new TTree("events", "outputTree");
	outputFile->cd();
	TTree* outputTree = inputTree->CloneTree(0);
	cout << "Cleaning events for " << inputPath << endl;

	vector<vector <Long64_t>*> 	timeVectors(3)				= {nullptr, nullptr, nullptr};
	vector<vector <Long64_t>*> 	timeVectorsCleaned(3)		= {nullptr, nullptr, nullptr};
	vector<vector <Float_t>*> 	energyVectors(3)			= {nullptr, nullptr, nullptr};
	vector<vector <Float_t>*> 	energyVectorsCleaned(3)		= {nullptr, nullptr, nullptr};
	vector<vector <Float_t>*> 	totVectors(3)				= {nullptr, nullptr, nullptr};
	vector<vector <Float_t>*> 	totVectorsCleaned(3) 		= {nullptr, nullptr, nullptr};
	vector<vector <UInt_t>*> 	channelIdVectors(3) 		= {nullptr, nullptr, nullptr};
	vector<vector <UInt_t>*> 	channelIdVectorsCleaned(3) 	= {nullptr, nullptr, nullptr};
	vector<vector <Int_t>*> 	xiVectors(3) 				= {nullptr, nullptr, nullptr};
	vector<vector <Int_t>*> 	xiVectorsCleaned(3) 		= {nullptr, nullptr, nullptr};
	const char* surfaces[3] = {"X","Y","U"};
	for (Int_t i = 0; i < 3; i++) {
		inputTree->SetBranchAddress(Form("time%s", surfaces[i]), &timeVectors[i]);
		outputTree->SetBranchAddress(Form("time%s", surfaces[i]), &timeVectorsCleaned[i]);
		inputTree->SetBranchAddress(Form("energy%s", surfaces[i]), &energyVectors[i]);
		outputTree->SetBranchAddress(Form("energy%s", surfaces[i]), &energyVectorsCleaned[i]);
		inputTree->SetBranchAddress(Form("tot%s", surfaces[i]), &totVectors[i]);
		outputTree->SetBranchAddress(Form("tot%s", surfaces[i]), &totVectorsCleaned[i]);
		inputTree->SetBranchAddress(Form("channelId%s", surfaces[i]), &channelIdVectors[i]);
		outputTree->SetBranchAddress(Form("channelId%s", surfaces[i]), &channelIdVectorsCleaned[i]);
		inputTree->SetBranchAddress(Form("xi%s", surfaces[i]), &xiVectors[i]);
		outputTree->SetBranchAddress(Form("xi%s", surfaces[i]), &xiVectorsCleaned[i]);
	}

	Long64_t entries = inputTree->GetEntries();

	for (Long64_t entry = 0; entry < entries; entry++) {
		if (entry % 10000 == 0 || entry == entries - 1) {
			cout << "\rEntry: " << entry << "/" << entries << flush;
		}

		inputTree->GetEntry(i);

		struct Entry {
			Float_t tot;
			Int_t surface;
		};

		vector<Entry> entries;

		for (int iv = 0; iv < 3; ++iv) {
			for (int i = 0; i < all[iv].size(); ++i) {
				entries.push_back({all[iv][i], iv, i});
			}
		}


		// group trigger entry as well
		// for (Int_t y = 0; y < 3; y++) {
		// 	timeVectors[y].push_back(time);
		// 	energyVectors[y].push_back(energy); //=5
		// 	totVectors[y].push_back(tot); //=0
		// 	channelIdVectors[y].push_back(channelId); //=4128
		// 	xiVectors[y].push_back(xi); //=0
		// }
		//
		// for (Int_t i = -1; i <= 1; i += 2) {
		// 	for (Long64_t j = entry + i; j >= 0 && j < entries; j += i) {
		// 		inputTree->GetEntry(j);
		// 		dt = time - time_tgr;
		//
		// 		if ((xi == 0 && energy == 5) || (xi == 0 && energy == -5)) {
		// 			break;
		// 		} else if ((i == -1 && dt > DT_RANGE[1]) || (i == 1 && dt < DT_RANGE[0])) {
		// 			continue;
		// 		} else if ((i == -1 && dt < DT_RANGE[0]) || (i == 1 && dt > DT_RANGE[1])) {
		// 			break;
		// 		}
		//
		// 		timeVectors[yi].push_back(time);
		// 		energyVectors[yi].push_back(energy);
		// 		totVectors[yi].push_back(tot);
		// 		channelIdVectors[yi].push_back(channelId);
		// 		xiVectors[yi].push_back(xi);
		// 	}
		// }
		//
		// vector<vector <size_t>> indices(3);
		// for (Int_t i = 0; i < 3; i++) {
		// 	for (size_t j = 0; j < totVectors[i].size(); j++) {
		// 		indices[i].push_back(j);
		// 	}
		//
		// 	vector<Long64_t> tot_sort = totVectors[i];
		// 	sort(indices[i].begin(), indices[i].end(), [&tot_sort](size_t i1, size_t i2) {
		// 		return tot_sort[i1] > tot_sort[i2];
		// 	});
		//
		// 	for (size_t j : indices[i]) {
		// 		timeVectorsCleaned[i].push_back(timeVectors[i][j]);
		// 		energyVectorsCleaned[i].push_back(energyVectors[i][j]);
		// 		totVectorsCleaned[i].push_back(totVectors[i][j]);
		// 		channelIdVectorsCleaned[i].push_back(channelIdVectors[i][j]);
		// 		xiVectorsCleaned[i].push_back(xiVectors[i][j]);
		// 	}
		// }

		// append gate entry to X vectors lastly
		timeVectorsCleaned[0].push_back((*timeVectors[0])[timeVectors[0]->size() - 1]);
		energyVectorsCleaned[0].push_back((*energyVectors[0])[energyVectors[0]->size() - 1]);
		totVectorsCleaned[0].push_back((*totVectors[0])[totVectors[0]->size() - 1]);
		channelIdVectorsCleaned[0].push_back((*channelIdVectors[0])[channelIdVectors[0]->size() - 1]);
		xiVectorsCleaned[0].push_back((*xiVectors[0])[xiVectors[0]->size() - 1]);

		outputTree->Fill();

		for (Int_t i = 0; i < 3; i++) {
			timeVectors[i].clear();
			timeVectorsCleaned[i].clear();
			energyVectors[i].clear();
			energyVectorsCleaned[i].clear();
			totVectors[i].clear();
			totVectorsCleaned[i].clear();
			channelIdVectors[i].clear();
			channelIdVectorsCleaned[i].clear();
			xiVectors[i].clear();
			xiVectorsCleaned[i].clear();
		}
	}

	outputFile->cd();
	outputTree->Write();
	outputFile->Close();
	inputFile->Close();

	cout << endl << "Saved to: " << output_path << endl;
}

