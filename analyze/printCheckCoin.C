#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <array>

#include <TTree.h>
#include <TH2F.h>
#include <TH1.h>
#include <TF1.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TMath.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

vector<TH1F*> hTotX(7);
vector<TH1F*> hTimingCenter(7);
vector<TH1F*> hXiX(7);
vector<TH2F*> hTimingXiX(7);

void printCheckCoinStart(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	gROOT->SetBatch(kTRUE);

	inData.tree->SetBranchStatus("coin", 1);
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	for (Int_t coin = 0; coin < 7; coin++) {
		delete hTotX[coin];
		hTotX[coin] = new TH1F(
			Form("hTotXCoin%d", coin),
			Form("totX (trigger = %d)", coin + 1),
			200, 0, totRange[1]
		);

		delete hXiX[coin];
		hXiX[coin] = new TH1F(
			Form("hXiXCoin%d", coin),
			Form("xiX (trigger = %d)", coin + 1),
			320, 0.5, 320.5
		);

		delete hTimingXiX[coin];
		hTimingXiX[coin] = new TH2F(
			Form("hTimingXiXCoin%d", coin),
			Form("xiX vs timing (trigger = %d); timing [ps]; xiX", coin + 1),
			200, timingRange[0], timingRange[1],
			320, 0.5, 320.5
		);

		delete hTimingCenter[coin];
		hTimingCenter[coin] = new TH1F(
			Form("hTimingCenterCoin%d", coin),
			Form("timing (xiX = 145~160) (trigger = %d)", coin + 1),
			200, timingRange[0], timingRange[1]
		);
	}
}

void printCheckCoinLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	// get single-coin
	Int_t coinOnes = 0;
	Int_t coinOne = -1;
	for (Int_t coin = 0; coin < 7; coin++) {
		if (inData.coin[coin] == 1) {
			coinOnes++;
			coinOne = coin;
		}
	}
	if (coinOnes != 1) {
		return;
	}

	// plot single-hits
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		// if (!inRange((*inData.totV[layer])[0], totRange)) {
		if ((*inData.totV[layer])[0] < 30e3) {
			return;
		}
		if (inData.xiV[layer]->size() > 1) {
			// if (inRange((*inData.totV[layer])[1], totRange)) {
			if ((*inData.totV[layer])[1] > 30e3) {
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

	// only use Z=1 (ToT = 100e3 ~ 120e3)
	if (!inRange((*inData.totV[0])[0], totRange)) {
		return;
	}

	hTotX[coinOne]->Fill((*inData.totV[0])[0]);
	hXiX[coinOne]->Fill((*inData.xiV[0])[0]);
	Long64_t timing = (*inData.timeV[0])[0] - (*inData.timeGate)[0];
	hTimingXiX[coinOne]->Fill(timing, (*inData.xiV[0])[0]);

	if (inRange((*inData.xiV[0])[0], {145, 160})) {
		hTimingCenter[coinOne]->Fill(timing);
	}
}

void printCheckCoinEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const TString& outName
) {
	TCanvas* c1 = nullptr;
	c1 = new TCanvas("c1", "c1", 800, 600);
	TString graphPath = Form("%s.pdf", outName.Data());
	c1->Print(Form("%s[", graphPath.Data()));
	c1->SetGrid();

	for (Int_t coin = 0; coin < 7; coin++) {
		addStats(hXiX[coin], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hXiX[coin]->GetEntries()), 
			Form("trigger = %d", coin),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
			"misalignment = {-5, 5}"
		});
		hXiX[coin]->Draw();
		c1->Print(graphPath);
	}

	for (Int_t coin = 0; coin < 7; coin++) {
		addStats(hTotX[coin], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTotX[coin]->GetEntries()), 
			Form("trigger = %d", coin),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
			"misalignment = {-5, 5}"
		});
		hTotX[coin]->Draw();
		c1->Print(graphPath);
	}

	for (Int_t coin = 0; coin < 7; coin++) {
		addStats(hTimingXiX[coin], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTimingXiX[coin]->GetEntries()), 
			Form("trigger = %d", coin),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
			"misalignment = {-5, 5}"
		});
		hTimingXiX[coin]->Draw();
		c1->SetLogz();
		c1->Print(graphPath);
		c1->SetLogz(0);
	}

	Double_t beamCoinPeak = 0;
	for (Int_t coin = 0; coin < 7; coin++) {
		Double_t peakMax = hTimingCenter[coin]->GetBinCenter(hTimingCenter[coin]->GetMaximumBin());
		TF1 gaus(Form("g_coin%d", coin), "gaus", peakMax - 3000, peakMax + 600);
		gaus.SetParameters(hTimingCenter[coin]->GetMaximum(), peakMax, hTimingCenter[coin]->GetRMS());
		hTimingCenter[coin]->Fit(&gaus, "QSR");
		Double_t peakMean = gaus.GetParameter(1);
		if (coin == 0) {
			beamCoinPeak = peakMean;
		}
		addStats(hTimingCenter[coin], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTimingCenter[coin]->GetEntries()), 
			Form("trigger = %d", coin),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
			"misalignment = {-5, 5}",
			Form("xiX = {%d, %d}", 145, 160),
			Form("amplitude = %.5g", gaus.GetParameter(0)),
			Form("mean [ps] = %.2f", gaus.GetParameter(1)),
			Form("sigma = %.5g", gaus.GetParameter(2)),
			Form("dt [ps] = %.2f", gaus.GetParameter(1) - beamCoinPeak)
		});
		c1->Print(graphPath);
	}

	c1->Print(Form("%s]", graphPath.Data()));
	delete c1;
}

void printCheckCoin(
	const vector<TString>& inPaths, 
	const Float_t (&totRange)[2] = {30e3, 1e6},
	const Long64_t (&timingRange)[2] = {-3530000, -3515000}
	// const Long64_t (&timingRange)[2] = {-5000, 5000}
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printCheckCoinStart(inData, totRange, timingRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printCheckCoinLoop(inData, totRange, timingRange);
	}

	TString outName = Form("%s_checkCoin", inData.runNum.Data());
	printCheckCoinEnd(inData, totRange, timingRange, outName);
}
