#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

TH1F* hdt = nullptr;
TH1F* hdxi = nullptr;
TH2F* hdt_dxi = nullptr;
TH2F* hdt_xi = nullptr;
TH2F* hdt_tot = nullptr;
TH2F* hdxi_tot = nullptr;
TH1F* hdt_digi = nullptr;
TH1F* hdt_fall = nullptr;

int time_diff(const char *fname) {
	TFile *inputFile = TFile::Open(fname);
	TTree *tree1 = (TTree*)inputFile->Get("data");

	// const Long64_t DT_RANGE[2]	= {-3000000, 3000000}; //ps
	// const Long64_t DT_RANGE[2]	= {-20000, 20000}; //ps
	const Long64_t DT_RANGE[2]	= {-6000000, 6000000}; //ps
	const Int_t XI_RANGE[2]		= {1, 320};
	const Float_t TOT_RANGE[2]	= {0, 300000};

	// TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	// c1->SetGrid();

	Long64_t dt_bins = (DT_RANGE[1] - DT_RANGE[0]) / 1000 * 4; //4 bin per 1ns
	hdt = new TH1F("hdt","hdt", dt_bins, DT_RANGE[0], DT_RANGE[1]);
	hdxi = new TH1F("hdxi","hdxi", XI_RANGE[1]-XI_RANGE[0]+1, XI_RANGE[0]-1-0.5, XI_RANGE[1]-1+0.5);
	hdt_xi = new TH2F("hdt_xi","hdt_xi", dt_bins, DT_RANGE[0], DT_RANGE[1], XI_RANGE[1]-XI_RANGE[0]+1, XI_RANGE[0]-0.5, XI_RANGE[1]+0.5);
	// hdt_tot = new TH2F("hdt_tot","hdt_tot", dt_bins, DT_RANGE[0], DT_RANGE[1], 100, TOT_RANGE[0], TOT_RANGE[1]);
	hdt_tot = new TH2F("hdt_tot","hdt_tot", dt_bins, DT_RANGE[0], DT_RANGE[1], 600, TOT_RANGE[0], TOT_RANGE[1]);
	hdt_dxi = new TH2F("hdt_dxi","hdt_dxi", dt_bins, DT_RANGE[0], DT_RANGE[1], XI_RANGE[1]-XI_RANGE[0]+1, XI_RANGE[0]-1-0.5, XI_RANGE[1]-1+0.5);
	hdxi_tot = new TH2F("hdxi_tot","hdxi_tot", XI_RANGE[1]-XI_RANGE[0]+1, XI_RANGE[0]-1-0.5, XI_RANGE[1]-1+0.5, 600, TOT_RANGE[0], TOT_RANGE[1]);
	hdt_digi = new TH1F("hdt","hdt", dt_bins, DT_RANGE[0], DT_RANGE[1]);
	hdt_fall = new TH1F("hdt","hdt", dt_bins, DT_RANGE[0], DT_RANGE[1]);

	// Set up variables to read from tree1
	Long64_t time; 
	UInt_t channelID;
	Float_t tot; 
	Float_t energy;
	Int_t xi;
	Int_t yi;
	tree1->SetBranchAddress("time", &time);
	tree1->SetBranchAddress("channelID", &channelID);
	tree1->SetBranchAddress("tot", &tot);
	tree1->SetBranchAddress("energy", &energy);
	tree1->SetBranchAddress("xi", &xi);
	tree1->SetBranchAddress("yi", &yi);

	// loop through all events
	Long64_t nentries = tree1->GetEntries();
	Int_t row = 0;
	Int_t xi_ref = 0;
	Int_t dxi = 0;
	Long64_t time_ref = 0;
	Long64_t dt = 0;

	// used for debug
	Long64_t freq = pow(10, floor(log10(nentries))-1);

	for (Long64_t i = 0; i < nentries; i++) {
		// print for debug
		if (i%freq == 0) {
			cout << i << endl;
		}

		tree1->GetEntry(i);
		if (!(xi==0 && energy==5)) {
		// if (!(tot >= 20000)) {
		// if (!(tot <= 10000)) {
			continue;
		}

		xi_ref = xi;
		time_ref = time;

		// look both ways
		for (Long64_t j=-1; j<=1; j=j+2) {
		// for (Long64_t j=1; j>=-1; j=j-3) {
			row = i;
			while (1) {
				row = row + j;
				if (row < 0 || row >= nentries) {
					break;
				}
				tree1->GetEntry(row);
				dt = time - time_ref;
				dxi = xi - xi_ref;

				if (!(dt >= DT_RANGE[0] && dt <= DT_RANGE[1])) {
					break;
				} else if (!(xi >= XI_RANGE[0] && xi <= XI_RANGE[1])) {
					continue;
				} else if (!(tot >= TOT_RANGE[0] && tot <= TOT_RANGE[1])) {
					continue;
				}
				// get digitized signal timing
				if (xi == 0 && yi == 63) {
					hdt_digi->Fill(dt);
					continue;
				}
				// get falling edge timing
				if (xi == 0 && energy == -5) {
					hdt_fall->Fill(dt);
					continue;
				}
				hdt->Fill(dt);
				hdxi->Fill(dxi);
				hdt_dxi->Fill(dt, dxi);
				hdt_xi->Fill(dt, xi);
				hdt_tot->Fill(dt, tot);
				hdxi_tot->Fill(dxi, tot);
			}
		}
	}

	return 0;
}
