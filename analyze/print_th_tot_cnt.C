#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TGraph.h>
#include <TCanvas.h>

#include <iostream>
#include <stdlib.h>

int print_th_tot_cnt() {
	// const TString DATA_DIR		= "/home/daq/daq_setup1/S10362-11-100P/th-rate";
	// const Float_t BD			= ;
	// const Float_t OV			= 1.2;

	// const TString DATA_DIR		= "/home/daq/daq_setup1/S13360-1375PE/th-rate_oldmapping";
	// const Float_t BD			= ;
	// const Float_t OV			= 8.0;

	const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1350PE/th-rate";
	const Float_t BD			= 51.83;
	const Float_t OV			= 4.0;

	const TString RAD 			= "bg";
	const UInt_t TH_ENUM[3]		= {10, 63, 1};
	const UInt_t TIME 			= 30; //seconds
	const UInt_t EXT 			= 0; //1: enable, 0: disable

	TString data_fname;
	TString graph_fname;
	if (EXT) {
		graph_fname = Form("%s/th-tot-cnt_%s_ov%0.1f_%ds_ext.pdf", DATA_DIR.Data(), RAD.Data(), OV, TIME);
	} else {
		graph_fname = Form("%s/th-tot-cnt_%s_ov%0.1f_%ds.pdf", DATA_DIR.Data(), RAD.Data(), OV, TIME);
	}

    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    c1->SetLogy();
	c1->Print(Form("%s[", graph_fname.Data()));

    for (UInt_t th=TH_ENUM[0]; th<=TH_ENUM[1]; th=th+TH_ENUM[2]) {
		if (EXT) {
			data_fname = Form(
				"%s/%s_ov%0.1f_th%d_%ds_ext.root", 
				DATA_DIR.Data(), RAD.Data(), OV, th, TIME
			);
		} else {
			data_fname = Form(
				"%s/%s_ov%0.1f_th%d_%ds.root", 
				DATA_DIR.Data(), RAD.Data(), OV, th, TIME
			);
		}
		cout << data_fname << endl;
		TFile *inputFile = TFile::Open(data_fname.Data());
		TTree *tree = (TTree*)inputFile->Get("data");

		Long64_t n_entries = tree->GetEntries();
		cout << "Entries: " << n_entries << endl;

		if (n_entries == 0) {
			TH1F *htot = new TH1F("htot", Form("threshold=%d",th), 500, -20000, 350000);
			htot->Draw();
		} else {
			tree->Draw("tot>>htot(500, -20000, 350000, 100, 1, 10000)", "xi==3");
			TH1F *htot = (TH1F*) gDirectory->Get("htot");
			htot->GetXaxis()->SetRangeUser(-20000, 350000);
			htot->GetYaxis()->SetRangeUser(1, 1000000);
			htot->SetTitle(Form("threshold=%d",th));
		}
		c1->Print(Form("%s", graph_fname.Data()));
		inputFile->Close();
	}
	c1->Print(Form("%s]", graph_fname.Data()));

	return 0;
}
