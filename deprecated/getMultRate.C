#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"

Long64_t* getMultRate(const vector<TString>& inPath, const UInt_t multArraySize, Int_t yi, vector <Float_t>& totRange, const vector <Float_t>& qRange, const vector <Float_t>& rateRange) {

	DataFBT5 inData(inPath, "events");
	const char* layers[3] = {"X","Y","U"};
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus(Form("tot%s", layers[yi]), 1);
	inData.tree->SetBranchStatus("l1q", 1);
	inData.tree->SetBranchStatus("l2q", 1);
	inData.tree->SetBranchStatus("rate", 1);
	for (Int_t i = 0; i < 3; i++) {
		inData.tree->SetBranchStatus(Form("xi%s", layers[i]), 1);
	}

	Long64_t *multArr = new Long64_t[multArraySize]();

	cout << "Total Events		: " << inData.entries << endl;
	UInt_t mult = 0;

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		Float_t charge = pow(inData.l1q * inData.l2q, 0.5);
		if (!(charge >= qRange[0] && charge < qRange[1])) continue;
		if (!(inData.rate >= rateRange[0] && inData.rate < rateRange[1])) continue;

		mult = 0;
		for (UInt_t entry = 0; entry < inData.totV[yi]->size(); entry++) {
			if ((*inData.totV[yi])[entry] >= totRange[0] && (*inData.totV[yi])[entry] < totRange[1]) {
				mult++;
			}
		}
		if (mult < multArraySize) {
			multArr[mult]++;
		} else {
			cout << endl << "Multiplicity array size too small for entry " << entry << endl;
			exit(1);
		}
	}
	cout << endl;

	return multArr;
}
