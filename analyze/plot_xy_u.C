#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TF2.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

TH2F* hxy_u = nullptr;
TH2F* hxy_u_shifted = nullptr;
TH2F* hxy_u_shifted_tot = nullptr;

int plot_xy_u(const char *fname) {
	TFile *inputFile = TFile::Open(fname);
	cout << fname << endl;
	TTree *events = (TTree*)inputFile->Get("events");

	hxy_u = new TH2F("hxy_u","hxy_u", 600, -0.5, 599.5, 320, -0.5, 319.5);
	hxy_u_shifted = new TH2F("hxy_shifted","hxy_shifted", 600, -300.5, 299.5, 320, -0.5, 319.5);
	hxy_u_shifted_tot = new TH2F("hxy_shifted_tot","hxy_shifted_tot", 600, -300.5, 299.5, 1000, 0, 500000);

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
	Long64_t xiX_xiY_shifted = 0;

	for (Long64_t i = 0; i < nentries; i++) {
		events->GetEntry(i);

		// xiX contains the gate entry so has to be > 1
		if (!(xiX->size() > 1 && xiY->size() > 0 && xiU->size() > 0)) {
			nskipped++;
			continue;
		}
		// if (!((*totU)[0] > 50000)) {
		// 	continue;
		// }

		xiX_xiY = (*xiX)[0] + (*xiY)[0];
		hxy_u->Fill(xiX_xiY, (*xiU)[0]);

		xiX_xiY_shifted = (*xiX)[0] + (*xiY)[0] - ((*xiU)[0] + 39.14285)/0.73142;
		hxy_u_shifted->Fill(xiX_xiY_shifted, (*xiU)[0]);

		hxy_u_shifted_tot->Fill(xiX_xiY_shifted, (*totU)[0]);
	}
	cout << "Skipped " << nskipped << "/" << nentries << endl;

	hxy_u->SetTitle(";xi_x+xi_y;xi_u");
	hxy_u_shifted->SetTitle(";(xi_x+xi_y)-(xi_u+30.4)/0.7;xi_u");
	hxy_u_shifted_tot->SetTitle(";(xi_x+xi_y)-(xi_u+30.4)/0.7;totU");

	return 0;
}
