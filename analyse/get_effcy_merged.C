#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "utils/get_mult_merged.C"

int get_effcy_merged(const char *fname, const UInt_t mult_arr_size, Int_t yi, vector <Float_t> tot_range, vector <Long64_t> q_range) {
	Long64_t *mult_arr = get_mult_merged(fname, mult_arr_size, yi, tot_range, q_range);
	Long64_t total = 0;
	cout << "Multiplicity(0~" << mult_arr_size << ")	: ";
	for (UInt_t i = 0; i < mult_arr_size; i++) {
		cout << mult_arr[i] << " ";
		total = total + mult_arr[i];
	}
	cout << endl << "Gate Entries		: " << total << endl;

	Float_t effcy;
	//return 0 if no entries
	if (total == 0) {
		cout << "Efficiency		: (none)\n" << endl;
		effcy = 0;
	} else {
		effcy = (Float_t)(total - mult_arr[0])/total;
		cout << "Efficiency		: " << effcy << "\n" << endl;
	}

	delete[] mult_arr;
	mult_arr = nullptr;

	return effcy;
}
