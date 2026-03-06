#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <iostream>
#include <stdlib.h>

TH2F *hTRtot1 = new TH2F(
	"hTRtot1","hTRtot1", 
	100, -5000, 5000, 100, 0, 400000
);
TH2F *hTRtot2 = new TH2F(
	"hTRtot2","hTRtot2", 
	100, -5000, 5000, 100, 0, 400000
);
TH2F *hTRtot1_shifted = new TH2F(
	"hTRtot1_shifted", "hTRtot1_shifted", 
	100, -5000, 5000, 100, 0, 400000
);
TH2F *hTRtot2_shifted = new TH2F(
	"hTRtot2_shifted", "hTRtot2_shifted", 
	100, -5000, 5000, 100, 0, 400000
);

int multiplicity(char *fname) {
    TFile *inputFile = TFile::Open(fname);
    TTree *tree1 = (TTree*)inputFile->Get("data");

    // Set up variables to read from tree1
    Long64_t time; 
    Float_t energy;
    Int_t xi;
    Float_t tot; 
    tree1->SetBranchAddress("time", &time);
    tree1->SetBranchAddress("energy", &energy);
    tree1->SetBranchAddress("xi", &xi);
    tree1->SetBranchAddress("tot", &tot);

    // Loop through entries in tree1 and write to tree2
    Int_t row = 0;
    Long64_t tpiv = 0; 
    Int_t multi = 0;
    Float_t E_fib = 0.;
    Float_t E_pla = 0.;

    Long64_t nentries = tree1->GetEntries();
    Long64_t time_start = 0;
    Long64_t time_piv = 0;

    // loop through all events
    for (Long64_t i = 0; i < nentries; i++) {
		// print for debug
		if (i%100000 == 0) {
			cout << i << endl;
		}
		// continue if not fiber1
		row = i;
		tree1->GetEntry(row);
		if (!(xi == 1 && tot >= 30000)) {
			continue;
		}
		// variables reset on event basis
		multi = 1;
		Double_t time_list[64] = {};
		Double_t tot_list[64] = {};
		tot_list[1] = tot;
		time_start = time;

		// look both ways
		for (Long64_t j=-1; j<=1; j=j+2) {
			while (1) {
				row = row + j;
				tree1->GetEntry(row);
				if (row < 0 || row >= nentries) {
					break;
				} else if (abs(time - time_start) > 10000) {
					break;
				} else if (xi>=2 && xi<=7 && tot>=30000) {
					multi++;
					time_list[xi] = time-time_start;
					tot_list[xi] = tot;
				}
			}
		}
		if (multi >= 7) {
			if (time_list[2] == 0) {
				continue;
			} else {
				hTRtot1->Fill(-1*time_list[2], tot_list[1]);
				hTRtot2->Fill(time_list[2], tot_list[2]);
			}
		}
    }

	TF2 *gaus_2d1 = new TF2(
		"gaus_2d1", 
		"[0]*exp(-((x - [1]*(y - [2]))^2 / (2*[3]^2)) - (y - [2])^2 / (2*[4]^2))", 
		-5000, 5000, 0, 300000
	);
	gaus_2d1->SetParameters(200, -1/50, 100000, 600, 30*1000);
	TF2 *gaus_2d2 = new TF2(
		"gaus_2d2", 
		"[0]*exp(-((x - [1]*(y - [2]))^2 / (2*[3]^2)) - (y - [2])^2 / (2*[4]^2))", 
		-5000, 5000, 0, 300000
	);
	gaus_2d2->SetParameters(200, -1/50, 100000, 600, 30*1000);
	hTRtot1->Fit(gaus_2d1);
	hTRtot2->Fit(gaus_2d2);
	
    // loop through all events
    for (Long64_t i = 0; i < nentries; i++) {
		// print for debug
		if (i%100000 == 0) {
			cout << i << endl;
		}
		// continue if not fiber1
		row = i;
		tree1->GetEntry(row);
		if (!(xi == 1 && tot >= 30000)) {
			continue;
		}
		// variables reset on event basis
		multi = 1;
		Double_t time_list[64] = {};
		Double_t tot_list[64] = {};
		tot_list[1] = tot;
		time_start = time;

		// look both ways
		for (Long64_t j=-1; j<=1; j=j+2) {
			while (1) {
				row = row + j;
				tree1->GetEntry(row);
				if (row < 0 || row >= nentries) {
					break;
				} else if (abs(time - time_start) > 10000) {
					break;
				} else if (xi>=2 && xi<=7 && tot>=30000) {
					multi++;
					tot_list[xi] = tot;
					time_list[xi] = time - time_start;
				}
			}
		}
		if (multi >= 7) {
			if (time_list[2] == 0) {
				continue;
			} else {
				Long64_t time_shift1 = (tot_list[1] - gaus_2d1->GetParameter(2))*gaus_2d1->GetParameter(1);
				Long64_t time_shift2 = (tot_list[2] - gaus_2d2->GetParameter(2))*gaus_2d2->GetParameter(1);
				hTRtot1_shifted->Fill(time_list[2] + time_shift1 - time_shift2, tot_list[1]);
				hTRtot2_shifted->Fill(time_list[2] + time_shift1 - time_shift2, tot_list[2]);
			}
		}
    }
    // Save and close the output file
    inputFile->Close();

    return 0;
}
