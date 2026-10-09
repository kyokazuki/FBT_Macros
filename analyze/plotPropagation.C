#include <TROOT.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMath.h>
#include <TF1.h>
#include <TH2F.h>

#include <iostream>
#include <array>
#include <stdlib.h>

#include "utils/addStats.C"
#include "utils/getSide.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
array<TH2F*, 4> hProp;
TH1D *py = nullptr;
TGraph* gPeaks = nullptr;
TF1* fitAll = nullptr;
// Int_t togaxsiWindow[3][2] = {{80, 230}, {43, 193}, {61, 261}};
Int_t togaxsiWindow[2][2] = {{80, 230}, {43, 193}};

void plotPropagationStart(const DataFBT2& inData) {
	gROOT->SetBatch(kTRUE);

	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
	}

	for (Int_t layer = 0; layer < 4; layer++) {
		delete hProp[layer];
		if (layer < 3) {
			hProp[layer] = new TH2F(
				Form("hProp%c", LAYER_NAMES[layer]),
				"timing vs distance;distance [mm];timing [ps]",
				320, -159.5 * 0.75, 160.5 * 0.75, 
				200, -5000, 5000
			);
		} else {
			hProp[layer] = new TH2F(
				"hPropAll",
				"timing vs distance;distance [mm];timing [ps]",
				320, -159.5 * 0.75, 160.5 * 0.75, 
				200, -5000, 5000
			);
		}
	}
}

void plotPropagationLoop(
	const DataFBT2& inData, 
	const Float_t (&totRange)[2]
) {
	// single-hit on XYU layers
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		if (!inRange((*inData.totV[layer])[0], totRange)) {
			return;
		}
		if (inData.xiV[layer]->size() > 1) {
			if ((*inData.totV[layer])[1] > totRange[0]) {
				return;
			}
		}
		// xiX and xiY window range
		if (layer < 2) {
			if (!inRange((*inData.xiV[layer])[0], togaxsiWindow[layer])) {
				return;
			}
		}
	}
	// filter out irrelevant hits
	if (!inRange(
		((*inData.xiV[0])[0] + (*inData.xiV[1])[0]) - ((*inData.xiV[2])[0] - MISALIGNMENT_OFFSET) / MISALIGNMENT_SLOPE, 
		{-5, 5}
	)) {
		return;
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		Double_t distance = getDistance(
			{(*inData.xiV[0])[0], (*inData.xiV[1])[0], (*inData.xiV[2])[0]}, 
			layer
		);
		hProp[layer]->Fill(
			distance,
			(*inData.timeV[layer])[0] - (*inData.timeGate)[0]
		);
		// only fill X and Y into hPropAll
		// if (layer == 2) {
		// 	break;
		// }
		hProp[3]->Fill(
			distance,
			(*inData.timeV[layer])[0] - (*inData.timeGate)[0]
		);
	}
}

void plotPropagationEnd(
	const DataBase& inData, 
	const Float_t (&totRange)[2],
	const TString& outName
) {
	array<TString, 4> graphPaths;
	for (Int_t layer = 0; layer < 4; layer++) {
		delete fitAll;
		delete gPeaks;
		gPeaks = new TGraph();

		if (layer < 3) {
			graphPaths[layer] = Form("%s%c.pdf", outName.Data(), LAYER_NAMES[layer]);
		} else {
			graphPaths[layer] = Form("%sAll.pdf", outName.Data());
		}

		c1->Print(Form("%s[", graphPaths[layer].Data()));

		// fit for each slice and print
		for (Int_t xbin = 1; xbin <= 320; xbin++) {
			py = hProp[layer]->ProjectionY(Form("py_x%d", xbin), xbin, xbin);
			py->Draw();

			if (py->GetEntries() <= 5000) {
				cout << "Not enough entries for bin " << xbin << "!" << endl;
				continue;
			} 

			// fit
			Double_t peakMax = py->GetBinCenter(py->GetMaximumBin());
			TF1 gaus(Form("g_x%d", xbin), "gaus", peakMax - 1000, peakMax + 500);
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
			gPeaks->SetPoint(gPeaks->GetN(), hProp[layer]->GetXaxis()->GetBinCenter(xbin), peak);

			c1->SetGrid();
			c1->Print(graphPaths[layer]);
			c1->Clear();

			delete py;
		}

		// fit the peaks
		gPeaks->Fit("pol1", "Q", "", -50, 50);
		fitAll = gPeaks->GetFunction("pol1");
		fitAll->SetLineWidth(1);
		Double_t intercept = fitAll->GetParameter(0);
		Double_t slope = fitAll->GetParameter(1);

		addStats(hProp[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hProp[layer]->GetEntries()), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("intercept = %.3f", intercept),
			Form("slope = %.3f", slope)
		});

		hProp[layer]->Draw();
		fitAll->Draw("SAME");
		gPeaks->Draw("PX SAME");

		c1->SetLeftMargin(0.15);
		c1->SetLogz();
		c1->SetGrid();
		c1->Print(graphPaths[layer]);
		c1->Print(Form("%s]", graphPaths[layer].Data()));
	}
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

	TString outName = Form("%s_propagation", inData.runNum.Data());
	plotPropagationEnd(inData, totRange, outName);
}

