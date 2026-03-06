#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

TH2F* hth_xi = nullptr;

int plot_th_xi_cnt() {
	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S10362-11-100P/th-rate";
	// const Float_t BD 			= ;
	// const Float_t OV		 	= 1.6;

	const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1375PE/th-rate_oldmapping";
	// const Float_t BD 			= ;
	const Float_t OV		 	= 8.0;

	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1350PE/th-rate";
	// const Float_t BD 			= 51.80;
	// const Float_t OV		 	= ;

	const TString RAD 			= "bg";
	const UInt_t TH_ENUM[3] 	= {0, 63, 1};
	const UInt_t TIME 			= 30; //seconds
	const UInt_t XI_RANGE[2] 	= {1, 32};

	Int_t th_count = (TH_ENUM[1] - TH_ENUM[0])/TH_ENUM[2] + 1;
	Int_t th;
	Int_t xi_count = XI_RANGE[1] - XI_RANGE[0] + 1;
	Int_t xi;
	TString data_fname;
	Long64_t entries;

	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	c1->SetGrid();
	TH1F* hxi_list[th_count];
	hth_xi = new TH2F(
		"hth-xi", "x: th, y: xi, z: count",
		th_count, TH_ENUM[0]-0.5, TH_ENUM[1]+0.5,
		xi_count, XI_RANGE[0]-0.5, XI_RANGE[1]+0.5
	);

	// loop through thresholds
	for (Int_t i=0; i<th_count; i++) {
		th = TH_ENUM[0] + i * TH_ENUM[2];

		// get tree from data file
		data_fname = Form(
			"%s/%s_ov%0.1f_th%d_%ds.root", 
			DATA_DIR.Data(), RAD.Data(), OV, th, TIME
		);
		TFile *inputFile = TFile::Open(data_fname.Data());
		cout << data_fname << endl;
		TTree *tree = (TTree*)inputFile->Get("data");
		entries = tree->GetEntries();
		cout << entries << " entries" << endl;

		// get xi graph from tree
		if (entries == 0) {
			continue;
		} else {
			tree->Draw(
				Form("xi>>hxi(%d, %f, %f)", xi_count, XI_RANGE[0]-0.5, XI_RANGE[1]+0.5), 
				"", "goff"
			);
			hxi_list[i] = (TH1F*) gDirectory->Get("hxi");

			// loop through xi and plot count
			for (Int_t j = 0; j < xi_count; j++) {
				xi = XI_RANGE[0] + j;
				Long64_t count = hxi_list[i]->GetBinContent(j+1);
				Int_t bin = hth_xi->FindBin(th, xi);
				hth_xi->SetBinContent(bin, count);
			}
		}
    }

	hth_xi->Draw("LEGO");
	// hth_xi->Draw("colz");
	
	return 0;
}
