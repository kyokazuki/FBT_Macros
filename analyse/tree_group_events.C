#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include <unistd.h>

void treeGroupEvents(const TString& input_path) {
	const Long64_t DT_RANGE[2] = {-285000, -250000};

	TString output_path = input_path;
	output_path.ReplaceAll(".root", "_grouped.root");
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

	vector<Long64_t> time_vector; 
	vector<Float_t> energy_vector;
	vector<Float_t> tot_vector;
	vector<UInt_t> channel_id_vector; 
	vector<Int_t> xi_vector;
	vector<Int_t> yi_vector;
	vector<Int_t> zi_vector;
	output_tree->Branch("time", &time_vector);
	output_tree->Branch("energy", &energy_vector);
	output_tree->Branch("tot", &tot_vector);
	output_tree->Branch("channelID", &channel_id_vector);
	output_tree->Branch("xi", &xi_vector);
	output_tree->Branch("yi", &yi_vector);

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

				time_vector.push_back(time);
				energy_vector.push_back(energy);
				tot_vector.push_back(tot);
				channel_id_vector.push_back(channel_id);
				xi_vector.push_back(xi);
				yi_vector.push_back(yi);
			}
		}
		output_tree->Fill();

		time_vector.clear();
		energy_vector.clear();
		tot_vector.clear();
		channel_id_vector.clear();
		xi_vector.clear();
		yi_vector.clear();
		zi_vector.clear();

		cout << endl;
	}

	output_file->cd();
	output_tree->Write();
	output_file->Close();
	input_file->Close();

	cout << "Saved to: " << output_path << endl;
}

