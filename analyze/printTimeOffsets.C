#include <iostream>
#include <stdlib.h>
#include <fstream>

#include <TTree.h>
#include <TH2F.h>
#include <TH1.h>
#include <TF1.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TCanvas.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

vector<TH2F*> hTimingXi(3);
TH1D* hTiming = nullptr;
vector<TGraph*> gTimingMeans(3);

void printTimeOffsetsStart(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	gROOT->SetBatch(kTRUE);

	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hTimingXi[layer];
		hTimingXi[layer] = new TH2F(
			Form("hTimingXi%c", LAYER_NAMES[layer]),
			Form(
				"xi%c vs timing (timeGate%%5000=1250);timing [ps];xi%c", 
				LAYER_NAMES[layer], LAYER_NAMES[layer]
			),
			200, timingRange[0], timingRange[1], 
			320, 0.5, 320.5
		);
	}
}

void printTimeOffsetsLoop(
	const DataFBT2& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	if ((*inData.timeGate)[0] % 5000 != 1250) {
		return;
	}

	// append fiber entries
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < inData.xiV[layer]->size(); hit++) {
			if (!inRange((*inData.totV[layer])[hit], totRange)) {
				continue;
			}
			Long64_t timing = (*inData.timeV[layer])[hit] - (*inData.timeGate)[0];
			if (!inRange(timing, timingRange)) {
				continue;
			}
			hTimingXi[layer]->Fill(
				timing,
				(*inData.xiV[layer])[hit]
			);
		}
	}
}

void printTimeOffsetsEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const TString& outName
) {
	TCanvas* c1 = nullptr;
	for (Int_t layer = 0; layer < 3; layer++) {
		c1 = new TCanvas("c1", "c1", 800, 600);
		TString graphPath = Form("%s%c.pdf", outName.Data(), LAYER_NAMES[layer]);
		c1->Print(Form("%s[", graphPath.Data()));

		ofstream out(Form("%s%c.tsv", outName.Data(), LAYER_NAMES[layer]));
		out << "xi\toffset\terror" << endl;

		gTimingMeans[layer] = new TGraph();

		for (Int_t ybin = 1; ybin <= LAYER_CHANNELS[layer]; ybin++) {
			hTiming = hTimingXi[layer]->ProjectionX(Form("hTiming_x%d", ybin), ybin, ybin);
			hTiming->Draw();

			if (hTiming->GetEntries() <= 0) {
				cout << "Not enough entries for layer " << LAYER_NAMES[layer] << " bin " << ybin << "!" << endl;
				// continue;
				return;
			} 

			Double_t peakMax = hTiming->GetBinCenter(hTiming->GetMaximumBin());
			TF1 gaus(Form("g_y%d", ybin), "gaus", peakMax - 5000, peakMax + 2000);
			gaus.SetParameters(hTiming->GetMaximum(), peakMax, hTiming->GetRMS());
			hTiming->Fit(&gaus, "QSR");
			Double_t peakMean = gaus.GetParameter(1);
			gTimingMeans[layer]->SetPoint(ybin, peakMean, ybin);
			out << ybin << "\t" 
				<< Form("%.0f", gaus.GetParameter(1) * -1) << "\t" 
				<< gaus.GetParError(1) << endl;

			c1->SetGrid();
			c1->Print(graphPath);

			delete hTiming;
		}

		addStats(hTimingXi[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hTimingXi[layer]->GetEntries()), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("timing = {%lld, %lld}", timingRange[0], timingRange[1])
		});
		hTimingXi[layer]->Draw();
		gTimingMeans[layer]->SetMarkerStyle(20);	// circle marker
		gTimingMeans[layer]->SetMarkerSize(0.2);
		gTimingMeans[layer]->SetMarkerColor(kRed);
		gTimingMeans[layer]->Draw("P SAME");
		c1->SetLogz();
		c1->SetGrid();
		c1->Print(graphPath);

		c1->Print(Form("%s]", graphPath.Data()));
		delete c1;
	}
}

void printTimeOffsets(
	const vector<TString>& inPaths, 
	const Float_t (&totRange)[2] = {50e3, 1e6},
	const Long64_t (&timingRange)[2] = {-3540000, -3488000}
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printTimeOffsetsStart(inData, totRange, timingRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printTimeOffsetsLoop(inData, totRange, timingRange);
	}

	TString outName = Form("%s_timeOffsets", inData.runNum.Data());
	printTimeOffsetsEnd(inData, totRange, timingRange, outName);
}
