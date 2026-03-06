#include <iostream>
#include <stdlib.h>

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>

#include "utils/printProgress.C"

Long64_t* getMultRate(TChain *chain, const UInt_t multArraySize, Int_t yi, vector <Float_t>& totRange, const vector <Float_t>& rateRange) {

	Long64_t *mult_arr = new Long64_t[multArraySize]();

	vector<Float_t>* tot = nullptr;
	vector <vector <Int_t>*> xiVectors(3);
	Float_t rate;
	const char* layers[3] = {"X","Y","U"};
	chain->SetBranchStatus("*", 0);
	chain->SetBranchStatus(Form("tot%s", layers[yi]), 1);
	chain->SetBranchAddress(Form("tot%s", layers[yi]), &tot);
	chain->SetBranchStatus("rate", 1);
	chain->SetBranchAddress("rate", &rate);
	for (Int_t i = 0; i < 3; i++) {
		chain->SetBranchStatus(Form("xi%s", layers[i]), 1);
		chain->SetBranchAddress(Form("xi%s", layers[i]), &xiVectors[i]);
	}

	Long64_t events = chain->GetEntries();
	cout << "Total Events		: " << events << endl;
	UInt_t mult = 0;

	for (Long64_t event = 0; event < events; event++) {
		printProgress(event, events);

		chain->GetEntry(event);

		if (!(rate >= rateRange[0] && rate < rateRange[1])) continue;

		// Float_t posAligned = (*xiVectors[0])[0] + (*xiVectors[1])[0] - ((*xiVectors[2])[0] + 39.14285)/0.73142;
		// if (!(posAligned >= -2 && posAligned <= 2)) continue;

		mult = 0;
		for (UInt_t entry = 0; entry < tot->size(); entry++) {
			if ((*tot)[entry] >= totRange[0] && (*tot)[entry] < totRange[1]) {
				mult++;
			}
		}
		if (mult < multArraySize) {
			mult_arr[mult]++;
		} else {
			cout << endl << "Multiplicity array size too small for event " << event << endl;
			exit(1);
		}
	}
	cout << endl;

	return mult_arr;
}
