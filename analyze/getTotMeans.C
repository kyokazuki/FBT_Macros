#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TMath.h>
#include <TROOT.h>
#include <TDirectory.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include "utils/parameters.C"
#include "utils/loadData.C"

const Float_t totBins 			= (TOT_RANGE[1] - TOT_RANGE[0]) / 1.e4 * 2.;
const Long64_t fitThreshold 	= 1000;

void getTotMeans(const TString& inPath) {
	gROOT->SetBatch(kTRUE);

	DataFBT1 inData({inPath}, "data");

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	cout << "Getting tot means from " << inPath << endl;

	TCanvas *c1 = nullptr;
	TH1D *py = nullptr;

	for (Int_t layer = 0; layer < 3; layer++) {
		const TString hName = Form("htot%s", LAYERS[layer]);
		const TString graph_fname = Form("totMeans%s_%s.pdf", LAYERS[layer], runNumber.Data());
		c1 = new TCanvas("c1", "c1", 800, 600);
		c1->Print(Form("%s[", graph_fname.Data()));

		inData.tree->Draw(
			Form(
				"tot:xi>>%s(%d, 0.5, %.1f, %.1f, %.1f, %.1f)", 
				hName.Data(), LAYER_CHANNELS[layer], LAYER_CHANNELS[layer] + 0.5, totBins, TOT_RANGE[0], TOT_RANGE[1]
			),
			Form("yi==%d", layer),
			"goff"
		);

		TH2* h2 = nullptr;
		gDirectory->GetObject(hName, h2);

		ofstream out(Form("totMeans%s_%s.tsv", LAYERS[layer], runNumber.Data()));
		out << "xi\tmean\terror" << endl;

		for (Int_t xbin = 1; xbin <= LAYER_CHANNELS[layer]; xbin++) {
			py = h2->ProjectionY(Form("py_x%d", xbin), xbin, xbin);

			if (py->GetEntries() <= fitThreshold) {
				out << xbin << "\t0\t0" << endl;
			} else {
				TF1 gaus(Form("g_y%d", xbin), "gaus", TOT_RANGE[0], TOT_RANGE[1]);
				gaus.SetParameters(py->GetMaximum(), py->GetMean(), py->GetRMS());

				auto r = py->Fit(&gaus, "QSR"); // quiet, save result, use range
				
				out << xbin << "\t" << gaus.GetParameter(1) << "\t" << gaus.GetParError(1) << endl;
			}

			c1->SetGrid();
			c1->Print(graph_fname);

			delete py;
		}
		c1->Print(Form("%s]", graph_fname.Data()));
		delete c1;
	}
}
