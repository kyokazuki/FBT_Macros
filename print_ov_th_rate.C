#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TGraph.h>
#include <TCanvas.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "/home/daq/macro/functions.C"

int print_ov_th_rate() {
	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S10362-11-100P/th-rate";
	// const Float_t BD 			= ;
	// const Float_t OV_ENUM[3] 	= {1.0, 2.0, 0.2};

	// const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1375PE/th-rate_oldmapping";
	// const Float_t BD 			= ;
	// const Float_t OV_ENUM[3] 	= {4.0, 8.0, 0.2};

	const TString DATA_DIR 		= "/home/daq/daq_setup1/S13360-1350PE/th-rate";
	const Float_t BD 			= 51.83;
	const Float_t OV_ENUM[3] 	= {0.0, 5.2, 0.2};

	const TString RAD 			= "bg";
	const UInt_t TH_ENUM[3]		= {10, 63, 1};
	const UInt_t TIME 			= 30; //seconds
	const UInt_t EXT 			= 0; //1: enable, 0: disable

	//condition for get_rate
	Double_t EXT_GATE_WIDTH		= 5e-6;
	Int_t XI_RANGE[2]			= {1, 32};
	Float_t TOT_RANGE[2]		= {30000, 500000};

	TString data_fname;
	TString graph_fname;
	if (EXT) {
		graph_fname = Form("%s/ov-th-rate_%s_%ds_ext.pdf", DATA_DIR.Data(), RAD.Data(), TIME);
	} else {
		graph_fname = Form("%s/ov-th-rate_%s_%ds.pdf", DATA_DIR.Data(), RAD.Data(), TIME);
	}

	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	c1->SetGrid();
	c1->SetLogy();
	c1->Print(Form("%s[", graph_fname.Data()));
	Long64_t rate = 0;

	for (Float_t ov=OV_ENUM[0]; ov<=OV_ENUM[1]; ov=ov+OV_ENUM[2]) {
		TGraph *gRate = new TGraph();
		gRate->SetTitle(Form("overvoltage = %0.1f", ov));
		gRate->GetXaxis()->SetRangeUser(0, TH_ENUM[1]+1);
		gRate->GetYaxis()->SetRangeUser(1, 300000);

		for (UInt_t th=TH_ENUM[0]; th<=TH_ENUM[1]; th=th+TH_ENUM[2]) {
			if (EXT) {
				data_fname = Form(
					"%s/%s_ov%0.1f_th%d_%ds_ext.root", 
					DATA_DIR.Data(), RAD.Data(), ov, th, TIME
				);
				rate = get_rate_ext(
					data_fname.Data(), EXT_GATE_WIDTH, 
					XI_RANGE[0], XI_RANGE[1], TOT_RANGE[0], TOT_RANGE[1]	
				);
			} else {
				data_fname = Form(
					"%s/%s_ov%0.1f_th%d_%ds.root", 
					DATA_DIR.Data(), RAD.Data(), ov, th, TIME
				);
				rate = get_rate(
					data_fname.Data(), TIME, 
					XI_RANGE[0], XI_RANGE[1], TOT_RANGE[0], TOT_RANGE[1]
				);
			}
			cout << data_fname << endl;
			cout << "Rate(xi=" << XI_RANGE[0] << "~" << XI_RANGE[1] << "): " << rate << endl;
			gRate->AddPoint(th, rate);
		}
		gRate->Draw("ALP");
		c1->Print(graph_fname);
	}

	c1->Print(Form("%s]", graph_fname.Data()));
	
	exit(0);
}
