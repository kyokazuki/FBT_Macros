#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

TH1F* hdt1 = nullptr;
TH1F* hdt2 = nullptr;
TH1F* hdt3 = nullptr;

int plot_time_res(const char *fname) {
	TFile *inputFile = TFile::Open(fname);
	cout << fname << endl;
	TTree *events = (TTree*)inputFile->Get("events");

	hdt1 = new TH1F("hdt1","hdt1", 1000, -10000, 30000);
	hdt2 = new TH1F("hdt2","hdt2", 500, -10000, 10000);
	hdt3 = new TH1F("hdt3","hdt3", 500, -10000, 10000);

	// Set up variables to read from tree1
	vector<Long64_t>* xiX = nullptr;
	vector<Long64_t>* timeX = nullptr;
	vector<Long64_t>* totX = nullptr;
	events->SetBranchAddress("xiX", &xiX);
	events->SetBranchAddress("timeX", &timeX);
	events->SetBranchAddress("totX", &totX);

	// loop through all events
	Long64_t nentries = events->GetEntries();
	Long64_t dt = 0;

	for (Long64_t i = 0; i < nentries; i++) {
		events->GetEntry(i);

		if (!(xiX->size() >= 2)) {
			continue;
		}

		if (!(abs((*xiX)[1] - (*xiX)[0]) == 1)) {
			continue;
		}
		hdt1->Fill((*timeX)[1] - (*timeX)[0]);

		if (!((*totX)[0] > 30000 && (*totX)[1] >30000)) {
			continue;
		}
		dt = (*timeX)[1] - (*timeX)[0];
		hdt2->Fill(dt);

		if (dt < 0) {
			hdt3->Fill(dt);
			hdt3->Fill(-dt);
		}
	}

	return 0;
}

