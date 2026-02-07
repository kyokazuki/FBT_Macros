#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TMath.h>
#include <TROOT.h>
#include <TDirectory.h>

#include <iostream>
#include <fstream>

void getTotMeans(const TString& inputPath) {
	TFile* inputFile = TFile::Open(inputPath, "READ");
	TTree* inputTree = (TTree*)inputFile->Get("data");
	cout << "Getting tot means from " << inputPath << endl;

	const Float_t TOT_RANGE[2] = {50000, 200000};
	const Int_t XI_RANGE[3] = {320, 224, 320};
	const char* LAYERS[3] = {"X", "Y", "U"};

	for (Int_t layer = 0; layer < 3; layer++) {
		const TString hName = Form("htot%s", LAYERS[layer]);

		inputTree->Draw(
			Form(
				"tot:xi>>%s(%d, 0.5, %.1f, 100, %.1f, %.1f)", 
				hname.Data(), XI_RANGE[layer], XI_RANGE[layer] + 0.5, TOT_RANGE[0], TOT_RANGE[1]
			),
			Form("yi==%d", layer),
			"goff"
		);

		TH2* h2 = nullptr;
		gDirectory->GetObject(hname, h2);

		ofstream out(Form("totMeans%c.tsv", LAYERS[layer]));
		out << "xi\tmean" << endl;

		for (Int_t xbin = 1; xbin < XI_RANGE[layer]; xbin++) {
			h2->ProjectionY(Form("py_x%d", xbin), xbin, xbin);

			TF1 gaus(Form("g_y%d", xbin), "gaus", TOT_RANGE[0], TOT_RANGE[1]);
		}
	}
}
