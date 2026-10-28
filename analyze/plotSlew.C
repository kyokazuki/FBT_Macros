#include <iostream>
#include <stdlib.h>

#include <TCanvas.h>
#include <TGraph.h>
#include <TMath.h>
#include <TF1.h>
#include <TH2F.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TCanvas *c1 = nullptr;
TH2F* hTotTiming = nullptr;
TH1D *py = nullptr;
TGraph* gPeaks = nullptr;
TF1* fitAll = nullptr;
TGraph* gTimeResTot = nullptr;

void plotSlewStart(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
	}

	delete c1;
	c1 = new TCanvas("c1", "c1", 800, 600);

	delete hTotTiming;
	hTotTiming = new TH2F(
		"hTimingTot",
		";tot [ps];timing [ps]", 
		200, 0, totRange[1],
		200, timingRange[0], timingRange[1]
	);

	delete gPeaks;
	gPeaks = new TGraph();

	delete gTimeResTot;
	gTimeResTot = new TGraph();
}

void plotSlewLoop(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	// plot single-hits
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		if (!inRange((*inData.totV[layer])[0], totRange)) {
			return;
		}
		if (inData.xiV[layer]->size() > 1) {
			if (inRange((*inData.totV[layer])[1], totRange)) {
				return;
			}
		}
	}

	// filter out irrelevant hits
	if (!inRange(
		((*inData.xiV[0])[0] + (*inData.xiV[1])[0]) - ((*inData.xiV[2])[0] - MISALIGNMENT_OFFSET) / MISALIGNMENT_SLOPE, {-5, 5}
	)) {
		return;
	}

	// detector center
	if (!inRange((*inData.xiV[0])[0], {145, 160})) {
		return;
	}

	hTotTiming->Fill(
		(*inData.totV[0])[0],
		(*inData.timeV[0])[0] - (*inData.timeGate)[0]	
	);
}

void plotSlewEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const TString& outName,
	Bool_t saveGraph
) {
	TString graphPath = Form("%s.pdf", outName.Data());
	if (saveGraph) {
		c1->Print(Form("%s[", graphPath.Data()));
	}

	for (Int_t xbin = 1; xbin <= 200; xbin++) {
		py = hTotTiming->ProjectionY(Form("py_x%d", xbin), xbin, xbin);
		py->Draw();

		if (py->GetEntries() <= 1000) {
			cout << "Not enough entries for bin " << xbin << "!" << endl;
			continue;
		} 

		// fit
		Double_t peakMax = py->GetBinCenter(py->GetMaximumBin());
		TF1 gaus(Form("g_x%d", xbin), "gaus", peakMax - 5000, peakMax + 1000);
		gaus.SetParameters(py->GetMaximum(), peakMax, py->GetRMS());
		py->Fit(&gaus, "QSR");

		// check fit results
		Double_t peak = gaus.GetParameter(1);
		if (peak <= 0 || gaus.GetParError(1) == 0) {
			cout << "Invalid fit for bin " << xbin << "!" << endl;
			cout << "Mean: " << peak << endl;
			cout << "Error: " << gaus.GetParError(1) << endl;
		}

		// plot to gPeaks
		gPeaks->SetPoint(
			gPeaks->GetN(), 
			hTotTiming->GetXaxis()->GetBinCenter(xbin),
			peak
		);

		// plot to gTimeResTot
		gTimeResTot->SetPoint(
			gTimeResTot->GetN(), 
			hTotTiming->GetXaxis()->GetBinCenter(xbin),
			gaus.GetParameter(2)
		);

		if (saveGraph) {
			c1->SetGrid();
			c1->Print(graphPath);
			c1->Clear();
		}

		delete py;
	}

	// fit the peaks
	// fitAll = new TF1("f", "[0]*(x - [1])*(x - [1])", timingRange[0], timingRange[1]);

	fitAll = new TF1("f", "[0]*exp([1]*x) + [2]", 0, totRange[1]);
	// fitAll->SetParameters(1e3, -2e-5, -100);
	fitAll->SetParameters(1e3, -2e-5, -3525e3);

	gPeaks->Fit(fitAll, "RN");
	fitAll->SetLineWidth(1);
	Double_t a = fitAll->GetParameter(0);
	Double_t b = fitAll->GetParameter(1);
	Double_t c = fitAll->GetParameter(2);

	addStats(hTotTiming, {
		Form("%s", inData.runNum.Data()),
		Form("entries = %.0f", hTotTiming->GetEntries()), 
		"trigger = 0",
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
		"misalignment = {-5, 5}",
		"xiX = {145, 160}"
		// "xiX = {145, 160}",
		// Form("a = %.6g", a),
		// Form("b = %.6g", b),
		// Form("c = %.6g", c)
	});

	hTotTiming->Draw();
	// fitAll->Draw("SAME");
	gPeaks->Draw("PX SAME");

	if (saveGraph) {
		c1->SetLeftMargin(0.15);
		c1->SetLogz();
		c1->SetGrid();
		c1->Print(graphPath);
		c1->Clear();
	}

	addStats(gTimeResTot, {
		Form("%s", inData.runNum.Data()),
		"trigger = 0",
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
		"misalignment = {-5, 5}",
		"xiX = {145, 160}"
	});
	gTimeResTot->SetMarkerStyle(20);
	gTimeResTot->SetMarkerSize(0.5);
	gTimeResTot->Draw();
	gTimeResTot->SetTitle("time resolution vs tot;tot [ps];sigma [ps]");

	if (saveGraph) {
		c1->SetGrid();
		c1->Print(graphPath);
		c1->Print(Form("%s]", graphPath.Data()));
	}
}

void plotSlew(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {30e3, 180e3},
	const Long64_t (&timingRange)[2] = {-3528000, -3518000},
	Bool_t saveGraph = 1
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotSlewStart(inData, totRange, timingRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotSlewLoop(inData, totRange, timingRange);
	}

	TString outName = Form("%s_slew", inData.runNum.Data());
	plotSlewEnd(inData, totRange, timingRange, outName, saveGraph);
}

