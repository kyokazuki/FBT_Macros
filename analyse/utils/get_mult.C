#include <iostream>
#include <stdlib.h>

#include <TFile.h>
#include <TTree.h>

Long64_t* get_mult(
	const char *fname, 
	const UInt_t mult_arr_size, 
	Int_t xi_range[], 
	Int_t yi_range[],
	Long64_t dt_range[], 
	Float_t tot_range[]
) {

	Long64_t *mult_arr = new Long64_t[mult_arr_size]();

	TFile *inputFile = TFile::Open(fname);
	cout << fname << endl;
	TTree *tree1 = (TTree*)inputFile->Get("data");

	// Set up variables to read from tree1
	Long64_t time; 
	Float_t energy;
	Float_t tot;
	Int_t xi;
	Int_t yi;
	tree1->SetBranchAddress("time", &time);
	tree1->SetBranchAddress("tot", &tot);
	tree1->SetBranchAddress("energy", &energy);
	tree1->SetBranchAddress("xi", &xi);
	tree1->SetBranchAddress("yi", &yi);

	// loop through all events
	Long64_t nentries = tree1->GetEntries();
	cout << "Total Entries		: " << nentries << endl;
	Long64_t row;
	Long64_t time_trigger;
	Long64_t dt;
	UInt_t mult;

	for (Long64_t i = 0; i < nentries; i++) {
		// skip if not ext
		tree1->GetEntry(i);
		if (!(xi == 0 && energy == 5)) {
			continue;
		}
		// variables reset on event basis
		time_trigger = time;
		mult = 0;
		// look both ways
		for (Int_t dirc = -1; dirc <= 1; dirc = dirc + 2) {
			row = i;

			while (1) {
				row = row + dirc;
				if (row < 0 || row >= nentries) {
					break;
				}

				tree1->GetEntry(row);
				dt = time - time_trigger;
				if (
					(dirc == -1 && dt < dt_range[0]) || 
					(dirc == 1 && dt > dt_range[1])
				) {
					break;
				} else if (
					(dirc == -1 && dt > dt_range[1]) || 
					(dirc == 1 && dt < dt_range[0])
				) {
					continue;
				} else if (
					(tot >= tot_range[0] && tot <= tot_range[1]) && 
					(xi >= xi_range[0] && xi <= xi_range[1]) &&
					(yi >= yi_range[0] && yi <= yi_range[1])
				) {
					mult++;
				}
			}
		}
		mult_arr[mult]++;
	}

	inputFile->Close();

	return mult_arr;
}
