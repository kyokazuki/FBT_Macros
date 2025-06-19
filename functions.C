#include <iostream>
#include <stdlib.h>
#include <math.h>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

Long64_t get_rate(
	const char *fname, 
	UInt_t time, 
	Int_t xi_min, Int_t xi_max, 
	Float_t tot_min, Float_t tot_max
) {

	Int_t xi_count = xi_max - xi_min + 1;
	Long64_t rate_avg = 0;

	TFile *inputFile = TFile::Open(fname);
	TTree *tree = (TTree*)inputFile->Get("data");
	if (tree->GetEntries() == 0) {
		inputFile->Close();
		return 0;
	}
	tree->Draw(
		"xi>>hxi", 
		Form(
			"xi>=%d && xi<=%d && tot>=%f && tot<=%f", 
			xi_min, xi_max, tot_min, tot_max
			), 
		"goff"
	);
	TH1F *hxi = (TH1F*) gDirectory->Get("hxi");

	for (Int_t i=1; i<=xi_count; i++) {
		rate_avg += hxi->GetBinContent(i);
	}
	rate_avg = (Long64_t) floor((Double_t) rate_avg / xi_count / time);

	inputFile->Close();

	return rate_avg;
}

Long64_t get_rate_ext(
	const char *fname, 
	Double_t ext_gate_width, 
	Int_t xi_min, Int_t xi_max, 
	Float_t tot_min, Float_t tot_max
) {

	Int_t xi_count = xi_max - xi_min + 1;
	Double_t live_time; 
	Double_t rate_avg;

	TFile *inputFile = TFile::Open(fname);
	TTree *tree = (TTree*)inputFile->Get("data");
	if (tree->GetEntries() == 0) {
		inputFile->Close();
		return 0;
	}
	tree->Draw(
		"xi>>hxi", 
		Form(
			"xi==0 || (xi>=%d && xi<=%d && tot>=%f && tot<=%f)", 
			xi_min, xi_max, tot_min, tot_max
			), 
		"goff"
	);
	TH1F *hxi = (TH1F*) gDirectory->Get("hxi");

	live_time = hxi->GetBinContent(1)/2 * ext_gate_width;
	if (live_time == 0) {
		return 0;
	}

	for (Int_t i=1; i<=xi_count; i++) {
		rate_avg += hxi->GetBinContent(i+1) / live_time;
	}
	rate_avg = floor(rate_avg / xi_count);

	inputFile->Close();

	return rate_avg;
}

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

Float_t get_effcy(const Long64_t mult_arr[], const UInt_t mult_arr_size) {
	Long64_t total = 0;
	cout << "Multiplicity(0~" << mult_arr_size << ")	: ";
	for (Int_t i = 0; i < mult_arr_size; i++) {
		cout << mult_arr[i] << " ";
		total = total + mult_arr[i];
	}
	cout << endl << "Gate Entries		: " << total << endl;

	//return 0 if no entries
	if (total == 0) {
		cout << "Efficiency		: (none)\n" << endl;
		return 0;
	} else {
		Float_t effcy = (Float_t)(total - mult_arr[0])/total;
		cout << "Efficiency		: " << effcy << "\n" << endl;
		return effcy;
	}
}
