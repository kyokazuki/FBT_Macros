#include <iostream>
#include <stdlib.h>

#include <TFile.h>
#include <TTree.h>

Long64_t* get_mult_merged(const char *fname, const UInt_t mult_arr_size, Int_t yi, vector <Float_t>& tot_range, vector <Long64_t> q_range) {

	Long64_t *mult_arr = new Long64_t[mult_arr_size]();

	TFile *inputFile = TFile::Open(fname);
	cout << fname << endl;
	TTree *events = (TTree*)inputFile->Get("events");

	// vector<vector<Long64_t>*> tot_vectors(3);
	// vector <string> surface = {"X", "Y", "U"};
	// for (Int_t i = 0; i < 3; i++) {
	// 	cout << i << endl;
	// 	events->SetBranchAddress(Form("tot%s", surface[i].c_str()), &tot_vectors[i]);
	// }
	vector<Long64_t>* tot;
	vector <string> surface = {"X", "Y", "U"};
	events->SetBranchAddress(Form("tot%s", surface[yi].c_str()), &tot);
	Double_t ulq, urq, dlq, drq;
	events->SetBranchAddress("ulq", &ulq);
	events->SetBranchAddress("urq", &urq);
	events->SetBranchAddress("dlq", &dlq);
	events->SetBranchAddress("drq", &drq);

	Long64_t nentries = events->GetEntries();
	cout << "Total Entries		: " << nentries << endl;
	Long64_t q;
	UInt_t mult = 0;

	for (Long64_t i = 0; i < nentries; i++) {
		events->GetEntry(i);

		q = pow(ulq*urq*dlq*drq, 0.25);
		if (!(q >= q_range[0] && q <= q_range[1])) {
			continue;
		}

		mult = 0;
		// for (Int_t j = 0; j < 3; j++) {
		// 	for (UInt_t k = 0; k < tot_vectors[j]->size(); k++) {
		// 		if ((*tot_vectors[j])[k] >= tot_range[0] && (*tot_vectors[j])[k] <= tot_range[1]) {
		// 			mult++;
		// 		}
		// 	}
		// }
		for (UInt_t k = 0; k < tot->size(); k++) {
			if ((*tot)[k] >= tot_range[0] && (*tot)[k] <= tot_range[1]) {
				mult++;
			}
		}
		mult_arr[mult]++;
	}

	inputFile->Close();

	return mult_arr;
}
