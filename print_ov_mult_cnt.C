#include <iostream>
#include <stdlib.h>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>

#include "/home/daq/macro/functions.C"

int print_ov_mult_cnt() {
	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S10362-11-100P/bias-effcy";
	// const Float_t BD 			= ;
	// const Float_t OV_ENUM[3] 	= {0.0, 4.0, 0.2};

	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1375PE/bias-effcy_oldmapping";
	// const Float_t BD 			= ;
	// const Float_t OV_ENUM[3] 	= {0.0, 10.0, 0.2};

	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1350PE/bias-effcy";
	const TString DATA_DIR 		= "/home/daq/daq_S13360-1350PE/setup1/bias-effcy";
	const Float_t BD 			= 51.83;
	const Float_t OV_ENUM[3] 	= {0.0, 6.0, 0.2};

	const TString RAD 			= "Sr90";
	const UInt_t TH 			= 50;
	const UInt_t TIME 			= 30; //seconds
	const UInt_t EXT 			= 1; //1: enable, 0: disable

	// conditions for multiplicity
	UInt_t MULT_ARR_SIZE 		= 8;
	UInt_t XI_RANGE[2] 			= {1, 7};
	Long64_t DT_RANGE[2] 		= {-475000, -455000};
	Float_t TOT_RANGE[2] 		= {30000, 500000};

	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	TH1F *hMult = new TH1F("hMult","hMult", 8, -0.5, 7.5);

	Float_t effcy;
	TString data_fname;
	TString graph_fname;
	if (EXT) {
		graph_fname = Form(
			"%s/ov-mult-cnt_%s_th%d_%ds_ext.pdf", 
			DATA_DIR.Data(), RAD.Data(), TH, TIME
		);
	} else {
		graph_fname = Form(
			"%s/ov-mult-cnt_%s_th%d_%ds.pdf", 
			DATA_DIR.Data(), RAD.Data(), TH, TIME
		);
	}
	c1->Print(Form("%s[", graph_fname.Data()));

	for (Float_t ov = OV_ENUM[0]; ov <= OV_ENUM[1]; ov = ov + OV_ENUM[2]) {
		// get data file
		if (EXT) {
			data_fname = Form(
				"%s/%s_ov%0.1f_th%d_%ds_ext.root", 
				DATA_DIR.Data(), RAD.Data(), ov, TH, TIME
			);
		} else {
			data_fname = Form(
				"%s/%s_ov%0.1f_th%d_%ds.root", 
				DATA_DIR.Data(), RAD.Data(), ov, TH, TIME
			);
		}

		Long64_t *mult_arr = get_mult(
			data_fname.Data(), MULT_ARR_SIZE, XI_RANGE, DT_RANGE, TOT_RANGE
		);
		for (Int_t i = 0; i < MULT_ARR_SIZE; i++) {
			hMult->SetBinContent(i + 1, mult_arr[i]);
		}
		effcy = get_effcy(mult_arr, MULT_ARR_SIZE)*100;
		delete[] mult_arr;
		mult_arr = nullptr;

		hMult->SetTitle(Form("%.1fV (efficiency %.1f%);multiplicity;entries", ov, effcy));
		hMult->Draw();
		c1->SetGrid();
		c1->Print(graph_fname);
		hMult->Reset();
	}

	c1->Print(Form("%s]", graph_fname.Data()));

	exit(0);
}
