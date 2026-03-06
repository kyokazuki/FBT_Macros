#include <iostream>
#include <stdlib.h>

Float_t get_effcy(const Long64_t mult_arr[], const UInt_t mult_arr_size) {
	Long64_t total = 0;
	cout << "Multiplicity(0~" << mult_arr_size << ")	: ";
	for (UInt_t i = 0; i < mult_arr_size; i++) {
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
