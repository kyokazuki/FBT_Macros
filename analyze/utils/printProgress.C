#include <iostream>

void printProgress(Long64_t entry, Long64_t entries) {
	// start from 1
	entry++;

	if (entry % 10000 == 0 || entry == entries) {
		Int_t progress = (Float_t) entry / (Float_t) entries * 100;
		cout << "\rProgress: " << entry << "/" << entries << " (" << progress << "%)";
	}

	if (entry == entries) {
		cout << endl;
	} else {
		cout << flush;
	}
}
