#include <TChain.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "getMultRate.C"

int getEffcyRate(const vector<TString>& inPath, const UInt_t multArrSize, Int_t yi, vector <Float_t> totRange, vector <Float_t> qRange, vector <Float_t> rateRange) {
	Long64_t *multArr = getMultRate(inPath, multArrSize, yi, totRange, qRange, rateRange);
	Long64_t total = 0;
	cout << "Multiplicity(0~" << multArrSize << ")	: ";
	for (UInt_t i = 0; i < multArrSize; i++) {
		cout << multArr[i] << " ";
		total = total + multArr[i];
	}
	cout << endl << "Events			: " << total << endl;

	Float_t effcy;
	//return 0 if no entries
	if (total == 0) {
		cout << "Efficiency		: (none)\n" << endl;
		effcy = 0;
	} else {
		effcy = (Float_t) (total - multArr[0]) / (Float_t) total;
		cout << "Efficiency		: " << effcy << "\n" << endl;
	}

	delete[] multArr;
	multArr = nullptr;

	return effcy;
}
