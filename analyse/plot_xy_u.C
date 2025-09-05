#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2F.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

TH2F* hxy_u = nullptr;

int plot_xy_u(char *fname) {
	TFile *inputFile = TFile::Open(fname);
	cout << fname << endl;
	TTree *events = (TTree*)inputFile->Get("events");

	hxy_u= new TH2F("hxy_u","hxy_u", 600, 0, 600, 320, 0, 320);

	vector<Long64_t>* xiX;
	vector<Long64_t>* xiY;
	vector<Long64_t>* xiU;
	events->SetBranchAddress("xiX", &xiX);
	events->SetBranchAddress("xiY", &xiY);
	events->SetBranchAddress("xiU", &xiU);

	// loop through all events
	Long64_t nentries = events->GetEntries();
	Long64_t nskipped = 0;

	for (Long64_t i = 0; i < nentries; i++) {
		events->GetEntry(i);

		if (xiY->size() > 0 && xiU->size() >0) {
			hxy_u->Fill((*xiX)[0] + (*xiY)[0], (*xiU)[0]);
		} else {
			nskipped++;
		}
	}
	cout << "Skipped " << nskipped << "/" << nentries;

	hxy_u->Draw();

	return 0;
}
