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

vector<TH1F*> hTiming(7);
vector<TH1F*> hTotX(7);
vector<TH1F*> hXiX(7);

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
		delete hTiming[coin];
		hTiming[coin] = new TH1F(
			Form("hTimingCoin%d", coin),
			Form("timing (xiX = 145~160) (coin = %d)", coin),
			200, timingRange[0], timingRange[1]
		);

		delete hTotX[coin];
		hTotX[coin] = new TH1F(
			Form("hTotXCoin%d", coin),
			Form("totX (coin = %d)", coin),
			200, 0, totRange[1]
		);

		delete hXiX[coin];
		hXiX[coin] = new TH1F(
			Form("hXiXCoin%d", coin),
			Form("xiX (coin = %d)", coin),
			320, 0.5, 320.5
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
	if (inData.xiV[0]->size() == 0) {
		return;
	}
	if (!inRange((*inData.totV[0])[0], totRange)) {
		return;
	}
	if (inData.xiV[0]->size() > 1) {
		if (inRange((*inData.totV[0])[1], totRange)) {
			return;
		}
	}
	hXiX[coinOne]->Fill((*inData.xiV[0])[0]);
	hTotX[coinOne]->Fill((*inData.totV[0])[0]);
	if (inRange((*inData.xiV[0])[0], {145, 160})) {
		hTiming[coinOne]->Fill((*inData.timeV[0])[0] - (*inData.timeGate)[0]);
	}

	// plot multiple hits
	// for (UInt_t hit = 0; hit < inData.xiV[0]->size(); hit++) {
	// 	if (!inRange((*inData.totV[0])[hit], totRange)) {
	// 		break;
	// 	}
	// 	hXiX[coinOne]->Fill((*inData.xiV[0])[hit]);
	// 	hTotX[coinOne]->Fill((*inData.totV[0])[hit]);
	// 	if (inRange((*inData.xiV[0])[hit], {145, 160})) {
	// 		hTiming[coinOne]->Fill((*inData.timeV[0])[hit] - (*inData.timeGate)[0]);
	// 	}
	// }
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
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hXiX[coin]->GetEntries()), 
			Form("coin = %d", coin),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1])
		});
		hXiX[coin]->Draw();
		c1->Print(graphPath);
	}

	for (Int_t coin = 0; coin < 7; coin++) {
		addStats(hTotX[coin], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hTotX[coin]->GetEntries()), 
			Form("coin = %d", coin),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1])
		});
		hTotX[coin]->Draw();
		c1->Print(graphPath);
	}

	Double_t beamCoinPeak = 0;
	for (Int_t coin = 0; coin < 7; coin++) {
		Double_t peakMax = hTiming[coin]->GetBinCenter(hTiming[coin]->GetMaximumBin());
		TF1 gaus(Form("g_coin%d", coin), "gaus", peakMax - 3000, peakMax + 600);
		gaus.SetParameters(hTiming[coin]->GetMaximum(), peakMax, hTiming[coin]->GetRMS());
		hTiming[coin]->Fit(&gaus, "QSR");
		Double_t peakMean = gaus.GetParameter(1);
		if (coin == 0) {
			beamCoinPeak = peakMean;
		}
		addStats(hTiming[coin], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hTotX[coin]->GetEntries()), 
			Form("coin = %d", coin),
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1]),
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
