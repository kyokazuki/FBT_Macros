#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>

#include <TTree.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TMath.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TCanvas.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TCanvas *c1 = nullptr;
TH2F* hXiTot = nullptr;
TH1D *py = nullptr;
TGraph* gPeaks = nullptr;

void printTotMeansStart(
	const DataFBT2& inData,
	Int_t layer
) {
	gROOT->SetBatch(kTRUE);

	inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
	inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);

	delete c1;
	c1 = new TCanvas("c1", "c1", 800, 600);

	delete hXiTot;
	hXiTot = new TH2F(
		Form("hXiTot%c", LAYER_NAMES[layer]),
		Form("tot vs xi (%c);xi;tot [ps]", LAYER_NAMES[layer]),
		MAX_XI_BINS, MAX_XI_RANGE[0] - 0.5, MAX_XI_RANGE[1] + 0.5,
		MAX_TOT_BINS / 2, MAX_TOT_RANGE[0], MAX_TOT_RANGE[1] / 2
	);
}

void printTotMeansLoop(
	const DataFBT2& inData,
	Int_t layer,
	Float_t minTot
) {
	for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
		if ((*inData.totV[layer])[hit] < minTot) {
			continue;
		}
		hXiTot->Fill(
			(*inData.xiV[layer])[hit],
			(*inData.totV[layer])[hit]
		);
	}
}

void printTotMeansEnd(
	const DataFBT2& inData,
	Int_t layer,
	Double_t minTot,
	const TString& outName,
	Bool_t saveGraph
) {
	TString graphPath = Form("%s%c.pdf", outName.Data(), LAYER_NAMES[layer]);
	if (saveGraph) {
		c1->Print(Form("%s[", graphPath.Data()));
	}

	ofstream out(Form("%s%c.tsv", outName.Data(), LAYER_NAMES[layer]));
	out << "xi\tmean\terror" << endl;

	gPeaks = new TGraph();

	for (Int_t xbin = 1; xbin <= LAYER_CHANNELS[layer]; xbin++) {
		py = hXiTot->ProjectionY(Form("py_x%d", xbin), xbin, xbin);
		py->Draw();

		if (py->GetEntries() <= 1000) {
			cout << "Not enough entries!" << endl;
			// return;
		} 

		// find peak mean
		Int_t peakBin = -1;
		Double_t threshold = py->GetMaximum() / 100;
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
		py->Fit(&gaus, "QSR");

		// fit twice
		peakMean = gaus.GetParameter(1);
		TF1 gaus2(Form("g2_y%d", xbin), "gaus", TMath::Max(peakMean - 30e3, minTot), peakMean + 100e3);
		gaus2.SetParameters(py->GetMaximum(), peakMean, py->GetRMS());
		py->Fit(&gaus2, "QSR");

		// check fit results
		Double_t peak = gaus2.GetParameter(1);
		if (peak <= 0 || gaus2.GetParError(1) == 0) {
			cout << "Invalid fit for layer " << LAYER_NAMES[layer] << " bin " << xbin << "!" << endl;
			cout << "Mean: " << peak << endl;
			cout << "Error: " << gaus2.GetParError(1) << endl;
			// return;
		}

		out << xbin << "\t" << peak << "\t" << gaus2.GetParError(1) << endl;

		// plot to gPeaks
		gPeaks->SetPoint(
			gPeaks->GetN(), 
			hXiTot->GetXaxis()->GetBinCenter(xbin),
			peak
		);

		if (saveGraph) {
			// c1->SetLogy();
			c1->SetGrid();
			c1->Print(graphPath);
			c1->Clear();
		}

		delete py;
	}

	addStats(hXiTot, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hXiTot->GetEntries()), 
		Form("tot = {%.3g, %.3g}", minTot, MAX_TOT_RANGE[1] / 2),
	});
	hXiTot->Draw();
	gPeaks->SetMarkerStyle(20);	// circle marker
	gPeaks->SetMarkerSize(0.2);
	gPeaks->SetMarkerColor(kRed);
	gPeaks->Draw("SAME PX");

	if (saveGraph) {
		c1->SetLogz();
		c1->SetGrid();
		c1->Print(graphPath);
		c1->Print(Form("%s]", graphPath.Data()));
	}
}

// X layer: minTot = 30e3 (xi = 224 has low ToT)
// Y layer: minTot = 30e3
// U layer: minTot = 40e3 (xi = 33 has high ToT, xi = 60 has low ToT)
void printTotMeans(
	const vector<TString>& inPaths,
	Int_t layer,
	Double_t minTot = 30e3,
	Bool_t saveGraph = 1
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printTotMeansStart(inData, layer);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printTotMeansLoop(inData, layer, minTot);
	}

	TString outName = Form("%s_totMeans", inData.runNum.Data());
	printTotMeansEnd(inData, layer, minTot, outName, saveGraph);
}

