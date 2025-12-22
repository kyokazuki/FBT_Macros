#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TF2.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

TH2F* hxy_u = nullptr;
TH2F* hxy_u_alignedU = nullptr;
TH2F* hxy_u_alignedY = nullptr;
TH2F* hxy_u_alignedYU = nullptr;
TH2F* hxy_u_shifted = nullptr;
TH1F* htot_peak = nullptr;
TH1F* htot_sides = nullptr;
TH1F* htot_low = nullptr;

int plot_xy_u(const char *fname) {
	TFile *inputFile = TFile::Open(fname);
	cout << fname << endl;
	TTree *events = (TTree*)inputFile->Get("events");

	hxy_u = new TH2F("hxy_u","hxy_u", 600, -0.5, 599.5, 320, -0.5, 319.5);
	hxy_u_alignedU = new TH2F("hxy_u_alignedU","hxy_u_alignedU", 600, -0.5, 599.5, 320, -0.5, 319.5);
	hxy_u_alignedY = new TH2F("hxy_u_alignedY","hxy_u_alignedY", 600, -0.5, 599.5, 320, -0.5, 319.5);
	hxy_u_alignedYU = new TH2F("hxy_u_alignedYU","hxy_u_alignedYU", 600, -0.5, 599.5, 320, -0.5, 319.5);
	hxy_u_shifted = new TH2F("hxy_shifted","hxy_shifted", 600, -300.5, 299.5, 320, -0.5, 319.5);
	htot_peak = new TH1F("htot_peak", "htot_peak", 200, 0, 500000);
	htot_sides = new TH1F("htot_sides", "htot_sides", 200, 0, 500000);
	htot_low = new TH1F("htot_low", "htot_low", 200, 0, 500000);

	vector<Long64_t>* xiX = nullptr;
	vector<Long64_t>* xiY = nullptr;
	vector<Long64_t>* xiU = nullptr;
	vector<Long64_t>* totU = nullptr;
	events->SetBranchAddress("xiX", &xiX);
	events->SetBranchAddress("xiY", &xiY);
	events->SetBranchAddress("xiU", &xiU);
	events->SetBranchAddress("totU", &totU);

	// loop through all events
	Long64_t nentries = events->GetEntries();
	Long64_t nskipped = 0;

	Long64_t xiX_xiY = 0;
	Long64_t xiY_aligned = 0;
	Long64_t xiU_aligned = 0;
	Long64_t xiX_xiY_shifted = 0;

	for (Long64_t i = 0; i < nentries; i++) {
		events->GetEntry(i);

		if (!(xiX->size() > 0 && xiY->size() > 0 && xiU->size() > 0)) {
			nskipped++;
			continue;
		}
		if (!((*totU)[0] > 30000)) {
			continue;
		}

		xiX_xiY = (*xiX)[0] + (*xiY)[0];
		hxy_u->Fill(xiX_xiY, (*xiU)[0]);

		if ((*xiY)[0] % 2 == 0) {
			xiY_aligned = (*xiY)[0] - 1;
		} else {
			xiY_aligned = (*xiY)[0] + 1;
		}
		if ((*xiU)[0] % 2 == 0) {
			xiU_aligned = (*xiU)[0] - 1;
		} else {
			xiU_aligned = (*xiU)[0] + 1;
		}
		hxy_u_alignedU->Fill(xiX_xiY, xiU_aligned);
		hxy_u_alignedY->Fill((*xiX)[0] + xiY_aligned, (*xiU)[0]);
		hxy_u_alignedYU->Fill((*xiX)[0] + xiY_aligned, xiU_aligned);
		
		xiX_xiY_shifted = (*xiX)[0] + (*xiY)[0] - (xiU_aligned + 30.4)/0.7;
		hxy_u_shifted->Fill(xiX_xiY_shifted, xiU_aligned);

		if (xiX_xiY >= 275 || xiX_xiY <= 225) {
			htot_sides->Fill((*totU)[0]);
		} else if (xiX_xiY_shifted >= -4 && xiX_xiY_shifted <= 4) {
			htot_peak->Fill((*totU)[0]);
		} else {
			htot_low->Fill((*totU)[0]);
		}
	}
	cout << "Skipped " << nskipped << "/" << nentries << endl;

	hxy_u->SetTitle(";xi_x+xi_y;xi_u");
	hxy_u_alignedU->SetTitle(";xi_x+xi_y;xi_u");
	hxy_u_alignedY->SetTitle(";xi_x+xi_y;xi_u");
	hxy_u_alignedYU->SetTitle(";xi_x+xi_y;xi_u");
	hxy_u_shifted->SetTitle(";(xi_x+xi_y)-(xi_u+30.4)/0.7;xi_u");

	return 0;
}
