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

void getTotMeans(const TString& inputPath) {
	gROOT->SetBatch(kTRUE);

	TFile* inputFile = TFile::Open(inputPath, "READ");
	TTree* inputTree = (TTree*)inputFile->Get("data");
	TString runNumber = TString(gSystem->BaseName(inputPath))(0,4);
	cout << "Getting tot means from " << inputPath << endl;

	const Float_t TOT_RANGE[2] = {50e3, 190e3};
	const Int_t XI_RANGE[3] = {320, 224, 320};
	const char* LAYERS[3] = {"X", "Y", "U"};
	// const Float_t totBins = (TOT_RANGE[1] - TOT_RANGE[0]) / 1.e4;
	const Float_t totBins = (TOT_RANGE[1] - TOT_RANGE[0]) / 1.e4 * 2.;

	TCanvas *c1 = nullptr;
	TH1D *py = nullptr;

	for (Int_t layer = 0; layer < 3; layer++) {
		const TString hName = Form("htot%s", LAYERS[layer]);
		const TString graph_fname = Form("totMeans%s_%s.pdf", LAYERS[layer], runNumber.Data());
		c1 = new TCanvas("c1", "c1", 800, 600);
		c1->Print(Form("%s[", graph_fname.Data()));

		inputTree->Draw(
			Form(
				"tot:xi>>%s(%d, 0.5, %.1f, %.1f, %.1f, %.1f)", 
				hName.Data(), XI_RANGE[layer], XI_RANGE[layer] + 0.5, totBins, TOT_RANGE[0], TOT_RANGE[1]
			),
			Form("yi==%d", layer),
			"goff"
		);

		TH2* h2 = nullptr;
		gDirectory->GetObject(hName, h2);

		ofstream out(Form("totMeans%s_%s.tsv", LAYERS[layer], runNumber.Data()));
		out << "xi\tmean\terror" << endl;

		for (Int_t xbin = 1; xbin <= XI_RANGE[layer]; xbin++) {
			py = h2->ProjectionY(Form("py_x%d", xbin), xbin, xbin);

			TF1 gaus(Form("g_y%d", xbin), "gaus", TOT_RANGE[0], TOT_RANGE[1]);
			gaus.SetParameters(py->GetMaximum(), py->GetMean(), py->GetRMS());

			auto r = py->Fit(&gaus, "QSR"); // quiet, save result, use range
			
			out << xbin << "\t" << gaus.GetParameter(1) << "\t" << gaus.GetParError(1) << endl;

			c1->SetGrid();
			c1->Print(graph_fname);

			delete py;
		}
		c1->Print(Form("%s]", graph_fname.Data()));
		delete c1;
	}
	inputFile->Close();
}
