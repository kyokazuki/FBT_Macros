#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <algorithm>
#include <stdlib.h>

#include <unistd.h>

void treeGroupEvents2(const TString& input_path) {
	const Long64_t DT_RANGE[2] = {-285000, -250000};

	TString output_path = input_path;
	output_path.ReplaceAll(".root", "_grouped2.root");
	TFile* input_file = TFile::Open(input_path);
	TFile* output_file = new TFile(output_path, "RECREATE");
	TTree* input_tree = (TTree*)input_file->Get("data");
	TTree* output_tree = new TTree("events", "output_tree");

	Long64_t time; 
	Float_t energy;
	Float_t tot;
	UInt_t channel_id; 
	Int_t xi;
	Int_t yi;
	input_tree->SetBranchAddress("time", &time);
	input_tree->SetBranchAddress("energy", &energy);
	input_tree->SetBranchAddress("tot", &tot);
	input_tree->SetBranchAddress("channelID", &channel_id);
	input_tree->SetBranchAddress("xi", &xi);
	input_tree->SetBranchAddress("yi", &yi);

	vector<vector <Long64_t>> time_vectors(3);
	vector<vector <Long64_t>> time_sorted_vectors(3);
	vector<vector <Long64_t>> energy_vectors(3);
	vector<vector <Long64_t>> energy_sorted_vectors(3);
	vector<vector <Long64_t>> tot_vectors(3);
	vector<vector <Long64_t>> tot_sorted_vectors(3);
	vector<vector <Long64_t>> channel_id_vectors(3);
	vector<vector <Long64_t>> channel_id_sorted_vectors(3);
	vector<vector <Long64_t>> xi_vectors(3);
	vector<vector <Long64_t>> xi_sorted_vectors(3);
	vector<string> surface = {"X", "Y", "U"};
	for (Int_t i = 0; i < 3; i++) {
		output_tree->Branch(Form("time%s", surface[i].c_str()), &time_sorted_vectors[i]);
		output_tree->Branch(Form("energy%s", surface[i].c_str()), &energy_sorted_vectors[i]);
		output_tree->Branch(Form("tot%s", surface[i].c_str()), &tot_sorted_vectors[i]);
		output_tree->Branch(Form("channelID%s", surface[i].c_str()), &channel_id_sorted_vectors[i]);
		output_tree->Branch(Form("xi%s", surface[i].c_str()), &xi_sorted_vectors[i]);
	}

	Long64_t n_entries = input_tree->GetEntries();
	Long64_t time_tgr;
	Long64_t dt;

	for (Long64_t entry = 0; entry < n_entries; entry++) {
		input_tree->GetEntry(entry);
		if (!(xi == 0 && energy == 5)) {
			continue;
		}

		time_tgr = time;

		for (Int_t i = -1; i <= 1; i += 2) {
			for (Long64_t j = entry + i; j >= 0 && j < n_entries; j += i) {
				input_tree->GetEntry(j);
				dt = time - time_tgr;
				cout << "Fbr	entry=" << j << "	time=" << time << "	dt=" << dt << "	tot=" << tot;

				if ((i == -1 && dt > DT_RANGE[1]) || (i == 1 && dt < DT_RANGE[0])) {
					cout << "	skipped" << endl;
					continue;
				} else if ((i == -1 && dt < DT_RANGE[0]) || (i == 1 && dt > DT_RANGE[1])) {
					cout << "	ended" << endl;
					break;
				}
				cout << "	filled" << endl;

				time_vectors[yi].push_back(time);
				energy_vectors[yi].push_back(energy);
				tot_vectors[yi].push_back(tot);
				channel_id_vectors[yi].push_back(channel_id);
				xi_vectors[yi].push_back(xi);
			}
		}

		vector<vector <size_t>> indices(3);
		for (Int_t i = 0; i < 3; i++) {
			for (size_t j = 0; j < tot_vectors[i].size(); j++) {
				indices[i].push_back(j);
			}

			vector<Long64_t> energy_sort = energy_vectors[i];
			sort(indices[i].begin(), indices[i].end(), [&energy_sort](size_t i1, size_t i2) {
				return energy_sort[i1] > energy_sort[i2];
			});

			for (size_t j : indices[i]) {
				time_sorted_vectors[i].push_back(time_vectors[i][j]);
				energy_sorted_vectors[i].push_back(energy_vectors[i][j]);
				tot_sorted_vectors[i].push_back(tot_vectors[i][j]);
				channel_id_sorted_vectors[i].push_back(channel_id_vectors[i][j]);
				xi_sorted_vectors[i].push_back(xi_vectors[i][j]);
			}
		}

		output_tree->Fill();

		for (Int_t i = 0; i < 3; i++) {
			time_vectors[i].clear();
			time_sorted_vectors[i].clear();
			energy_vectors[i].clear();
			energy_sorted_vectors[i].clear();
			tot_vectors[i].clear();
			tot_sorted_vectors[i].clear();
			channel_id_vectors[i].clear();
			channel_id_sorted_vectors[i].clear();
			xi_vectors[i].clear();
			xi_sorted_vectors[i].clear();
		}

		cout << endl;
	}

	output_file->cd();
	output_tree->Write();
	output_file->Close();
	input_file->Close();

	cout << "Saved to: " << output_path << endl;
}

