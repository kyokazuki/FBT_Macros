#include <iostream>
#include <stdlib.h>
#include <math.h>

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TGraph.h>

#include "/home/daq/macro/functions.C"

TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
TGraph *gEffcy = new TGraph();

int plot_ov_effcy() {
	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S10362-11-100P/bias-effcy";
	// const Float_t BD 			= 70.75;
	// const Float_t OV_ENUM[3] 	= {0.0, 4.0, 0.2};
	// const UInt_t TIME 			= 30; //seconds

	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1375PE/ov-effcy_oldmapping";
	// const Float_t BD 			= 51.83;
	// const Float_t OV_ENUM[3] 	= {0.0, 10.0, 0.2};
	// const UInt_t TIME 			= 30; //seconds

	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1350PE/ov-effcy";
	// const Float_t BD 			= 51.83;
	// const Float_t OV_ENUM[3] 	= {0.0, 6.0, 0.2};
	// const UInt_t TIME 			= 30; //seconds

	// const TString DATA_DIR 		= "/home/daq/daq_setup2/S13360-1350PE/ov-effcy";
	// const TString DATA_DIR 		= "/home/daq/daq_setup2/S13360-1350PE_uncalibrated/ov-effcy";
	// const Float_t BD 			= 51.80;
	// const Float_t OV_ENUM[3] 	= {1.0, 6.0, 0.2};
	// const UInt_t TIME 			= 600; //seconds

	// const TString DATA_DIR 		= "/home/daq/daq_setup5/beamtime/heavy";
	const TString DATA_DIR 		= "/home/daq/daq_setup5/beamtime/light/ov-effcy";
	// const TString DATA_DIR 		= "/home/daq/daq_setup5/beamtime/light2/ov-effcy_th40";
	const Float_t BD 			= 51.80;
	const Float_t OV_ENUM[3] 	= {1.2, 3.0, 0.2};
	// const Float_t OV_ENUM[3] 	= {2.2, 3.6, 0.2};
	const UInt_t TIME 			= 900; //seconds

	const TString RAD 			= "light";
	const UInt_t TH 			= 20;
	const UInt_t EXT 			= 1; //1: enable, 0: disable

	// conditions for multiplicity
	UInt_t MULT_ARR_SIZE 		= 32; //expected largest multiplicity value
	Int_t XI_RANGE[2] 			= {1, 320};
	Int_t YI_RANGE[2]		 	= {0, 0};
	// Long64_t DT_RANGE[2] 		= {-475000, -455000}; //prototype
	// Long64_t DT_RANGE[2] 		= {-265000, -245000}; //deployed
	Long64_t DT_RANGE[2] 		= {-280000, -250000}; //deployed
	Float_t TOT_RANGE[2] 		= {30000, 500000}; //default: >30k
	// Float_t TOT_RANGE[2] 		= {20000, 500000}; //default: >30k

	TString data_fname;
	Float_t effcy;

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
			data_fname.Data(), MULT_ARR_SIZE, XI_RANGE, YI_RANGE, DT_RANGE, TOT_RANGE
		);
		effcy = get_effcy(mult_arr, MULT_ARR_SIZE);
		delete[] mult_arr;
		mult_arr = nullptr;

		gEffcy->AddPoint(ov, effcy);
	}
	
	gEffcy->Draw("ALP");
	gEffcy->GetXaxis()->SetRangeUser(OV_ENUM[0], OV_ENUM[1] + OV_ENUM[2]);
	gEffcy->GetYaxis()->SetRangeUser(0, 1);
	gEffcy->SetTitle(Form(";voltage - %.2f[V];efficiency", BD));
	c1->SetGrid();
	c1->Draw();
	
	return 0;
}
