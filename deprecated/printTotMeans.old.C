#include <iostream>
#include <stdlib.h>
#include <fstream>

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

#include "utils/loadData.C"
#include "utils/zoomAxis.C"

// X layer: minTot = 30e3, ratio = 100 (xi = 224 has low ToT)
// Y layer: minTot = 30e3, ratio = 100
// U layer: minTot = 40e3, ratio = 100 (xi = 33 has high ToT, xi = 60 has low ToT)
void printTotMeans(const vector<TString>& inPaths, Int_t layer, Double_t minTot = 30e3, Int_t ratio = 100) {
	gROOT->SetBatch(kTRUE);

	DataFBT1 inData(inPaths, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	TCanvas *c1 = nullptr;
	TH1D *py = nullptr;

	// for (Int_t layer = 0; layer < 3; layer++) {
	const TString graphName = Form("htot%c", LAYER_NAMES[layer]);
	const TString graphPath = Form("%s_totMeans%c.pdf", inData.runNum.Data(), LAYER_NAMES[layer]);
	c1 = new TCanvas("c1", "c1", 800, 600);
	c1->Print(Form("%s[", graphPath.Data()));

	inData.tree->Draw(
		Form(
			"tot:xi>>%s(%d, 0.5, %.1f, %d, %e, %e)", 
			graphName.Data(), LAYER_CHANNELS[layer], LAYER_CHANNELS[layer] + 0.5, 
			MAX_TOT_BINS / 2, minTot, MAX_TOT_RANGE[1] / 2
		),
		Form("yi==%d", layer),
		"goff"
	);

	TH2* hTot = nullptr;
	gDirectory->GetObject(graphName, hTot);

	ofstream out(Form("%s_totMeans%c.tsv", inData.runNum.Data(), LAYER_NAMES[layer]));
	out << "xi\tmean\terror" << endl;

	for (Int_t xbin = 1; xbin <= LAYER_CHANNELS[layer]; xbin++) {
		py = hTot->ProjectionY(Form("py_x%d", xbin), xbin, xbin);
		py->Draw();

		if (py->GetEntries() <= 1000) {
			cout << "Not enough entries!" << endl;
			// return;
		} 

		// find peak mean
		Int_t peakBin = -1;
		Double_t threshold = py->GetMaximum() / ratio;
		for (Int_t bin = py->GetNbinsX(); bin >= 2; bin--) {
			Double_t y = py->GetBinContent(bin);

			if (y > threshold && y > 1 && y > py->GetBinContent(bin-1) && y > py->GetBinContent(bin+1)) {
				peakBin = bin;
				break;
			}
		}
		if (peakBin == -1) {
			cout << "Couldn't find peak bin!" << endl;
			// return;
		}
		Float_t peakMean = py->GetBinCenter(peakBin);
		cout << "Found peak mean " << peakMean << endl;

		// fit
		TF1 gaus(Form("g_y%d", xbin), "gaus", minTot, peakMean + 100e3);
		gaus.SetParameters(py->GetMaximum(), peakMean, py->GetRMS());
		auto r = py->Fit(&gaus, "QSR");

		// fit twice
		peakMean = gaus.GetParameter(1);
		TF1 gaus2(Form("g2_y%d", xbin), "gaus", TMath::Max(peakMean - 30e3, minTot), peakMean + 100e3);
		gaus2.SetParameters(py->GetMaximum(), peakMean, py->GetRMS());
		r = py->Fit(&gaus2, "QSR");

		// check fit results
		if (gaus2.GetParameter(1) <= 0 || gaus2.GetParError(1) == 0) {
			cout << "Invalid fit for layer " << LAYER_NAMES[layer] << " bin " << xbin << "!" << endl;
			cout << "Mean: " << gaus2.GetParameter(1) << endl;
			cout << "Error: " << gaus2.GetParError(1) << endl;
			// return;
		}

		out << xbin << "\t" << gaus2.GetParameter(1) << "\t" << gaus2.GetParError(1) << endl;

		c1->SetLogy();
		c1->SetGrid();
		c1->Print(graphPath);

		delete py;
	}
	c1->Print(Form("%s]", graphPath.Data()));
	delete c1;
	// }
}
