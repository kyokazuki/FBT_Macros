#include <iostream>
#include <stdlib.h>

#include <TFile.h>
#include <TTree.h>

Long64_t* getMultGrouped(const char *fname, const UInt_t mult_arr_size, Int_t yi, vector <Float_t>& tot_range) {

	Long64_t *mult_arr = new Long64_t[mult_arr_size]();

	TFile *inputFile = TFile::Open(fname);
	cout << fname << endl;
	TTree *events = (TTree*)inputFile->Get("events");

	vector<Long64_t>* tot = nullptr;
	vector <string> surface = {"X", "Y", "U"};
	events->SetBranchAddress(Form("tot%s", surface[yi].c_str()), &tot);

	Long64_t nentries = events->GetEntries();
	cout << "Total Entries		: " << nentries << endl;
	UInt_t mult = 0;

	for (Long64_t i = 0; i < nentries; i++) {
		events->GetEntry(i);

		mult = 0;
		for (UInt_t k = 0; k < tot->size(); k++) {
			if ((*tot)[k] >= tot_range[0] && (*tot)[k] <= tot_range[1]) {
				mult++;
			}
		}
		if (mult < mult_arr_size) {
			mult_arr[mult]++;
		} else {
			cout << "Multiplicity array size too small!" << endl;
			exit(1);
		}
	}

	inputFile->Close();

	return mult_arr;
}
