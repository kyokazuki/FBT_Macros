#include <TROOT.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMath.h>
#include <TF1.h>
#include <TH2F.h>

#include <iostream>
#include <stdlib.h>

#include "utils/addStats.C"
#include "utils/getSide.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
TGraph* gPropAll = nullptr;
TH2F* hPropAll = nullptr;
TH1D *py = nullptr;
TGraph* gPeaks = nullptr;
TF1* fitAll = nullptr;

void plotPropagationStart(const DataFBT2& inData) {
	gROOT->SetBatch(kTRUE);

	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
	}

	delete gPropAll;
	gPropAll = new TGraph();

	delete hPropAll;
	hPropAll = new TH2F(
		"hPropAll",
		";;",
		320, -159.5, 160.5, 
		// 200, -20000, 30000
		200, -5000, 5000
	);

	delete gPeaks;
	gPeaks = new TGraph();
}

void plotPropagationLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.xiV[layer]->size() < 2) {
			return;
		}
		if (!inRange((*inData.totV[layer])[0], totRange)) {
			return;
		}
		// remove multi-hits
		if (inRange((*inData.totV[layer])[1], totRange)) {
			return;
		}
	}
	// filter out irrelevant hits
	if (!inRange(((*inData.xiV[0])[0] + (*inData.xiV[1])[0]) - ((*inData.xiV[2])[0] - POS_OFFSET) / POS_SLOPE, {-10, 10})) {
		return;
	}

	for (Int_t layer = 0; layer < 2; layer++) {
		Int_t distance = getDistance({(*inData.xiV[0])[0], (*inData.xiV[1])[0]}, layer);
		gPropAll->SetPoint(
			gPropAll->GetN(), 
			distance,
			(*inData.timeV[layer])[0] - (*inData.timeGate)[0]
		);
		hPropAll->Fill(
			distance,
			(*inData.timeV[layer])[0] - (*inData.timeGate)[0]
		);
	}
}

void plotPropagationEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2]
) {
	TString graphPath = Form("%s_propagation.pdf", inData.runNum.Data());

	// fit for each slice and print
	c1->Print(Form("%s[", graphPath.Data()));
	for (Int_t xbin = 1; xbin <= 320; xbin++) {
		c1->Clear();

		py = hPropAll->ProjectionY(Form("py_x%d", xbin), xbin, xbin);
		py->Draw();

		if (py->GetEntries() <= 10000) {
			cout << "Not enough entries for bin " << xbin << "!" << endl;
			continue;
		} 

		// fit
		Double_t peakMax = py->GetBinCenter(py->GetMaximumBin());
		TF1 gaus(Form("g_x%d", xbin), "gaus", peakMax - 1000, peakMax + 1000);
		gaus.SetParameters(py->GetMaximum(), peakMax, py->GetRMS());
		py->Fit(&gaus, "QSR");

		// check fit results
		Double_t peak = gaus.GetParameter(1);
		if (peak <= 0 || gaus.GetParError(1) == 0) {
			cout << "Invalid fit for bin " << xbin << "!" << endl;
			cout << "Mean: " << peak << endl;
			cout << "Error: " << gaus.GetParError(1) << endl;
			// return;
		}

		// plot to gPeaks
		gPeaks->SetPoint(gPeaks->GetN(), hPropAll->GetXaxis()->GetBinCenter(xbin), peak);

		c1->SetGrid();
		c1->Print(graphPath);

		delete py;
	}

	// fit the peaks
	gPeaks->Fit("pol1", "Q");
	fitAll = gPeaks->GetFunction("pol1");
	fitAll->SetLineWidth(1);
	Double_t intercept = fitAll->GetParameter(0);
	Double_t slope = fitAll->GetParameter(1);

	addStats(hPropAll, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %d", gPropAll->GetN()), 
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("intercept = %.3f", intercept),
		Form("slope = %.3f", slope)
	});

	hPropAll->Draw();
	fitAll->Draw("SAME");
	gPeaks->Draw("PX SAME");

	c1->SetLogz();
	c1->SetGrid();
	c1->Print(graphPath);

	c1->Print(Form("%s]", graphPath.Data()));
}

void plotPropagation(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2]		= {0.5, 10.0}
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotPropagationStart(inData);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotPropagationLoop(inData, totRange);
	}

	plotPropagationEnd(inData, totRange);
}

