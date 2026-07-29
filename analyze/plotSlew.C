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

void plotSlewStart(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete c1;
	c1 = new TCanvas("c1", "c1", 800, 600);

	delete hTotTiming;
	hTotTiming = new TH2F(
		"hTimingTot",
		"timing vs tot;tot;timing [ps]", 
		200, 0, totRange[1],
		200, timingRange[0], timingRange[1]
	);

	delete gPeaks;
	gPeaks = new TGraph();
}

void plotSlewLoop(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			if (!inRange((*inData.totV[layer])[hit], totRange)) {
				continue;
			}
			Long64_t timing = (*inData.timeV[layer])[hit] - (*inData.timeGate)[0];
			if (!inRange(timing, timingRange)) {
				continue;
			}
			hTotTiming->Fill(
				(*inData.totV[layer])[hit],
				timing
			);
		}
	}
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
			// return;
		}

		// plot to gPeaks
		gPeaks->SetPoint(
			gPeaks->GetN(), 
			hTotTiming->GetXaxis()->GetBinCenter(xbin),
			// hTimingTot->GetYaxis()->GetBinCenter(ybin)
			peak
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

	// fitAll = new TF1("f", "[0]*exp([1]*x) + [2]", timingRange[0], timingRange[1]);
	// fitAll->SetParameters(100e3, -0.01, 30e3);
	// fitAll = new TF1("f", "exp([0]*(x - [1])) + [2]", 0, totRange[1]);
	// fitAll->SetParameters(-2e-5, 400e3, -100);
	fitAll = new TF1("f", "[0]*exp([1]*x) + [2]", 0, totRange[1]);
	fitAll->SetParameters(1e3, -2e-5, -100);

	// fitAll = new TF1("f", "[0]/(x - [1]) + [2]", timingRange[0], timingRange[1]);
	// fitAll->SetParameters(200e6, -2000, 0);

	// fitAll = new TF1("f", "[0]*log(x - [1]) + [2]", timingRange[0], timingRange[1]);
	// fitAll->SetParameters(-600e3, -6e3, 2400e3);

	gPeaks->Fit(fitAll, "RN");
	fitAll->SetLineWidth(1);
	Double_t a = fitAll->GetParameter(0);
	Double_t b = fitAll->GetParameter(1);
	Double_t c = fitAll->GetParameter(2);

	addStats(hTotTiming, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hTotTiming->GetEntries()), 
		Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
		Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
		Form("a = %.4g", a),
		Form("b = %.4g", b),
		Form("c = %.4g", c)
	});

	hTotTiming->Draw();
	fitAll->Draw("SAME");
	gPeaks->Draw("PX SAME");

	if (saveGraph) {
		c1->SetLogz();
		c1->SetGrid();
		c1->Print(graphPath);
		c1->Print(Form("%s]", graphPath.Data()));
	}
}

void plotSlew(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {0.5, 10.0},
	const Long64_t (&timingRange)[2] = {-10000, 20000},
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

