#ifndef PRINTPROGRESS_C
#define PRINTPROGRESS_C

#include <iostream>

void printProgress(Long64_t entry, Long64_t entries) {
	// start from 1
	entry++;

	Long64_t freq = std::max<Long64_t>(100, entries / 100);

	if (entry % freq == 0 || entry == entries) {
		Int_t progress = (Float_t) entry / (Float_t) entries * 100;
		cout << "Progress: " << entry << "/" << entries << " (" << progress << "%)\r" << flush;
	}

	if (entry == entries) {
		cout << endl;
	} else {
		cout << flush;
	}
}

#endif
