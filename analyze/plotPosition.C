#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TF2.h>
#include <TChain.h>
#include <TGraph.h>
#include <TSystem.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include "utils/loadData.C"
#include "utils/printProgress.C"

const Float_t totRange[2] 			= {50e3, 180e3};
const Float_t qRange[2] 			= {0, 600};

const Float_t posRange[2] = {-300.5, 299.5};
const Float_t posBins = 600;
const Int_t totBins = 500;
const Int_t rateBins = 50;
// const Float_t rateRange[2] = {0.0e6, 8.0e6};
const Float_t rateRange[2] = {0.0e6, 8.0e6};

TH2F* hPos = nullptr;
TH2F* hPosAligned = nullptr;
TH2F* hPosAlignedDt = nullptr;
TH2F* hPosAlignedTotX = nullptr;
TH2F* hPosAlignedTotY = nullptr;
TH2F* hPosAlignedTotU = nullptr;
TH2F* hPosAlignedGateX = nullptr;
TH2F* hPosAlignedGateY = nullptr;
TH2F* hPosAlignedGateU = nullptr;
TH2F* hPosAlignedRate = nullptr;
TH2F* hPosAlignedRateNorm = nullptr;
TGraph* gRatePosAligned = nullptr;

void plotPosition(const vector<TString>& inPath) {
	// DataFBT2 inData(inPath, "events");
	// DataFBT4 inData(inPath, "events");
	DataFBT5 inData(inPath, "events");

	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("l1q", 1);
	inData.tree->SetBranchStatus("l2q", 1);
	// inData.tree->SetBranchStatus("m1q", 1);
	// inData.tree->SetBranchStatus("m2q", 1);
	// inData.tree->SetBranchStatus("s1q", 1);
	inData.tree->SetBranchStatus("timeGate", 1);
	inData.tree->SetBranchStatus("rate", 1);
	const char* layers[3] = {"X","Y","U"};
	for (size_t i = 0; i < 3; i++) {
		inData.tree->SetBranchStatus(Form("time%s", layers[i]), 1);
		inData.tree->SetBranchStatus(Form("tot%s", layers[i]), 1);
		inData.tree->SetBranchStatus(Form("xi%s", layers[i]), 1);
	}

	delete hPos;
	delete hPosAligned;
	delete hPosAlignedDt;
	delete hPosAlignedTotX;
	delete hPosAlignedTotY;
	delete hPosAlignedTotU;
	delete hPosAlignedGateX;
	delete hPosAlignedGateY;
	delete hPosAlignedGateU;
	delete hPosAlignedRate;
	delete hPosAlignedRateNorm;
	delete gRatePosAligned;

	hPos = new TH2F("hPos","hPos", posBins, -0.5, 599.5, 320, -0.5, 319.5);
	hPosAligned = new TH2F("hPosAligned","hPosAligned", posBins, posRange[0], posRange[1], 320, -0.5, 319.5);
	hPosAlignedDt = new TH2F("hPosAlignedDt","hPosAlignedDt", posBins, posRange[0], posRange[1], 100, 0, 50e3);
	hPosAlignedTotX = new TH2F("hPosAlignedTotX","hPosAlignedTotX", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedTotY = new TH2F("hPosAlignedTotY","hPosAlignedTotY", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedTotU = new TH2F("hPosAlignedTotU","hPosAlignedTotU", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedGateX = new TH2F("hPosAlignedGateX","hPosAlignedGateX", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	hPosAlignedGateY = new TH2F("hPosAlignedGateY","hPosAlignedGateY", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	hPosAlignedGateU = new TH2F("hPosAlignedGateU","hPosAlignedGateU", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	hPosAlignedRate = new TH2F("hPosAlignedRate","hPosAlignedRate", posBins, posRange[0], posRange[1], rateBins, rateRange[0], rateRange[1]);
    gRatePosAligned = new TGraph();

	// loop through all inData.tree
	Long64_t nskipped = 0;
	Long64_t pos = 0;
	Long64_t posAligned = 0;
	Long64_t dt = 0;

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		// xiX contains the gate entry so has to be > 1
		if (!(inData.xiV[0]->size() > 0 && inData.xiV[1]->size() > 0 && inData.xiV[2]->size() > 0)) {
			nskipped++;
			continue;
		}
		// if (!((*inData.timeV[0])[0] < (*inData.timeV[1])[0] && (*inData.timeV[1])[0] < (*inData.timeV[2])[0])) {
		// 	nskipped++;
		// 	continue;
		// }
		if (!(
			(*inData.totV[0])[0] >= totRange[0] && (*inData.totV[0])[0] < totRange[1] &&
			(*inData.totV[1])[0] >= totRange[0] && (*inData.totV[1])[0] < totRange[1] &&
			(*inData.totV[2])[0] >= totRange[0] && (*inData.totV[2])[0] < totRange[1]
		)) {
			nskipped++;
			continue;
		}
		if (!(pow(inData.l1q * inData.l2q, 0.5) >= qRange[0] && pow(inData.l1q * inData.l2q, 0.5) < qRange[1])) {
			nskipped++;
			continue;
		} 
		// if (!(pow(inData.m1q * inData.m2q, 0.5) >= qRange[0] && pow(inData.m1q * inData.m2q, 0.5) < qRange[1])) {
		// 	nskipped++;
		// 	continue;
		// } 
		// if (!(s1q >= qRange[0] && s1q < qRange[1])) {
		// 	nskipped++;
		// 	continue;
		// } 

		pos = (*inData.xiV[0])[0] + (*inData.xiV[1])[0];
		posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] + 39.14285)/0.73142;
		dt = abs((*inData.timeV[0])[0] - (*inData.timeV[1])[0]) + abs((*inData.timeV[1])[0] - (*inData.timeV[2])[0]) + abs((*inData.timeV[2])[0] - (*inData.timeV[0])[0]);

		hPos->Fill(pos, (*inData.xiV[2])[0]);
		hPosAligned->Fill(posAligned, (*inData.xiV[2])[0]);
		hPosAlignedDt->Fill(posAligned, dt);
		hPosAlignedTotX->Fill(posAligned, (*inData.totV[0])[0]);
		hPosAlignedTotY->Fill(posAligned, (*inData.totV[1])[0]);
		hPosAlignedTotU->Fill(posAligned, (*inData.totV[2])[0]);
		hPosAlignedGateX->Fill(posAligned, (*inData.timeV[0])[0] - (*inData.timeGate)[0]);
		hPosAlignedGateY->Fill(posAligned, (*inData.timeV[1])[0] - (*inData.timeGate)[0]);
		hPosAlignedGateU->Fill(posAligned, (*inData.timeV[2])[0] - (*inData.timeGate)[0]);
		hPosAlignedRate->Fill(posAligned, inData.rate);
	}

	// normalize hPosAlignedRate
	Int_t ipoint = 0;
	Float_t rateSteps = (rateRange[1] - rateRange[0]) / (Float_t)rateBins;
	hPosAlignedRateNorm = (TH2F*) hPosAlignedRate->Clone("hPosAlignedRateNorm");
	for (Int_t rateBin = 1; rateBin <= rateBins; rateBin++) {
		Float_t stripIntegral = 0.0;
		for (Int_t posBin = 1; posBin <= posBins; posBin++) {
			stripIntegral += hPosAlignedRate->GetBinContent(posBin, rateBin);
		}
		if (stripIntegral == 0) continue;
		for (Int_t posBin = 1; posBin <= posBins; posBin++) {
			Float_t value = hPosAlignedRate->GetBinContent(posBin, rateBin);
			hPosAlignedRateNorm->SetBinContent(posBin, rateBin, value / stripIntegral);
		}
		Float_t posAlignedIntegral = 0.0;
		for (Int_t posBin = posBins / 2 - 1; posBin <= posBins / 2 + 4; posBin++) {
			posAlignedIntegral += hPosAlignedRate->GetBinContent(posBin, rateBin);
		}
		gRatePosAligned->SetPoint(
			ipoint, 
			rateSteps * ((Float_t)rateBin - 0.5), 
			posAlignedIntegral / stripIntegral
		);
		ipoint++;
	}

	hPosAlignedRateNorm = (TH2F*) hPosAlignedRate->Clone("hPosAlignedRateNorm");
	for (Int_t rateBin = 1; rateBin <= rateBins; rateBin++) {
		Float_t stripIntegral = 0.0;
		for (Int_t posBin = 1; posBin <= posBins; posBin++) {
			stripIntegral += hPosAlignedRate->GetBinContent(posBin, rateBin);
		}
		if (stripIntegral == 0) continue;
		for (Int_t posBin = 1; posBin <= posBins; posBin++) {
			Float_t value = hPosAlignedRate->GetBinContent(posBin, rateBin);
			hPosAlignedRateNorm->SetBinContent(posBin, rateBin, value / stripIntegral);
		}
		Float_t posAlignedIntegral = 0.0;
		for (Int_t posBin = posBins / 2 - 3; posBin <= posBins / 2 + 6; posBin++) {
			posAlignedIntegral += hPosAlignedRate->GetBinContent(posBin, rateBin);
		}
		gRatePosAligned->SetPoint(rateBin, (Float_t)(rateRange[1] - rateRange[0]) / (Float_t)rateBins * ((Float_t)rateBin - 0.5), posAlignedIntegral / stripIntegral);
	}

	hPos->SetTitle(";xiX+xiY;xiU");
	hPosAligned->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;xiU");
	hPosAlignedDt->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;sum of all fiber event time differences");
	hPosAlignedTotX->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;totX");
	hPosAlignedTotY->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;totY");
	hPosAlignedTotU->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;totU");
	hPosAlignedGateX->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;timeX-timeGate");
	hPosAlignedGateY->SetTitle(";(xiY+xiY)-(xiU+30.4)/0.7;timeY-timeGate");
	hPosAlignedGateU->SetTitle(";(xiU+xiY)-(xiU+30.4)/0.7;timeU-timeGate");

	hPosAlignedRate->SetTitle(";(xiU+xiY)-(xiU+30.4)/0.7;rate");
	hPosAlignedRateNorm->SetTitle(";(xiU+xiY)-(xiU+30.4)/0.7;rate normalized");

	gRatePosAligned->SetTitle(";rate [Hz];alignment (+/-4)");
	gRatePosAligned->SetMarkerStyle(20);	// circle marker
	gRatePosAligned->SetMarkerSize(0.5);
	gRatePosAligned->SetMarkerColor(kRed);
	gRatePosAligned->GetXaxis()->SetLimits(rateRange[0], rateRange[1]);
	gRatePosAligned->GetYaxis()->SetRangeUser(0.0, 1.0);

	if (inPath.size() == 1) {
		TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
		c1->SetGrid();
		TString runNumber = TString(gSystem->BaseName(inPath[0]))(0,4);
		TString graphPath = Form("%s_ratePosAligned.pdf", runNumber.Data());
		gRatePosAligned->Draw();
		c1->Print(graphPath);
		delete c1;
	}

	cout << "Skipped " << nskipped << "/" << inData.entries << endl;
}
