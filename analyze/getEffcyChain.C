#include <TChain.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "utils/getMultChain.C"
#include "initChain.C"

int getEffcyChain(TChain *chain, const UInt_t mult_arr_size, Int_t yi, vector <Float_t> tot_range, vector <Float_t> rate_range) {
	Long64_t *mult_arr = getMultChain(chain, mult_arr_size, yi, tot_range, rate_range);
	Long64_t total = 0;
	cout << "Multiplicity(0~" << mult_arr_size << ")	: ";
	for (UInt_t i = 0; i < mult_arr_size; i++) {
		cout << mult_arr[i] << " ";
		total = total + mult_arr[i];
	}
	cout << endl << "Events			: " << total << endl;

	Float_t effcy;
	//return 0 if no entries
	if (total == 0) {
		cout << "Efficiency		: (none)\n" << endl;
		effcy = 0;
	} else {
		effcy = (Float_t) (total - mult_arr[0]) / (Float_t) total;
		cout << "Efficiency		: " << effcy << "\n" << endl;
	}

	delete[] mult_arr;
	mult_arr = nullptr;

	return effcy;
}
