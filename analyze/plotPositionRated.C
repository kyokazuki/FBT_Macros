#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TF2.h>
#include <TChain.h>
#include <TGraph.h>

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include "utils/initChain.C"
#include "utils/printProgress.C"

TH2F* hPos = nullptr;
TH2F* hPosAligned = nullptr;
TH2F* hPosAlignedDt = nullptr;
TH2F* hPosAlignedTotX = nullptr;
TH2F* hPosAlignedTotY = nullptr;
TH2F* hPosAlignedTotU = nullptr;
TH2F* hPosAlignedGateX = nullptr;
TH2F* hPosAlignedGateY = nullptr;
TH2F* hPosAlignedGateU = nullptr;
// for chain
TH2F* hPosAlignedRate = nullptr;
TH2F* hPosAlignedRateNorm = nullptr;
TGraph* gRatePosAligned = nullptr;

void plotPositionRated(TChain* inputTree) {
	const Float_t totRange[2] 			= {50e3, 400e3};
	const Float_t qRange[2] 			= {0, 100};

	vector <vector <Long64_t>*> timeVectors(3);
	vector <vector <Int_t>*> xiVectors(3);
	vector <vector <Float_t>*> totVectors(3);
	Double_t m1q, m2q, s1q;
	vector <Long64_t>* timeGate = nullptr;
	inputTree->SetBranchStatus("*", 0);
	const char* layers[3] = {"X","Y","U"};
	// inputTree->SetBranchStatus("m1q", 1);
	// inputTree->SetBranchAddress("m1q", &m1q);
	// inputTree->SetBranchStatus("m2q", 1);
	// inputTree->SetBranchAddress("m2q", &m2q);
	// inputTree->SetBranchStatus("s1q", 1);
	// inputTree->SetBranchAddress("s1q", &s1q);
	inputTree->SetBranchStatus("timeGate", 1);
	inputTree->SetBranchAddress("timeGate", &timeGate);
	for (Int_t i = 0; i < 3; i++) {
		inputTree->SetBranchStatus(Form("time%s", layers[i]), 1);
		inputTree->SetBranchAddress(Form("time%s", layers[i]), &timeVectors[i]);
		inputTree->SetBranchStatus(Form("tot%s", layers[i]), 1);
		inputTree->SetBranchAddress(Form("tot%s", layers[i]), &totVectors[i]);
		inputTree->SetBranchStatus(Form("xi%s", layers[i]), 1);
		inputTree->SetBranchAddress(Form("xi%s", layers[i]), &xiVectors[i]);
	}
	// for chain
	Float_t rate;
	inputTree->SetBranchStatus("rate", 1);
	inputTree->SetBranchAddress("rate", &rate);

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

	const Float_t posRange[2] = {-300.5, 299.5};
	const Float_t posBins = posRange[1] - posRange[0];
	const Int_t totBins = 500;
	hPos = new TH2F("hPos","hPos", posBins, -0.5, 599.5, 320, -0.5, 319.5);
	hPosAligned = new TH2F("hPosAligned","hPosAligned", posBins, posRange[0], posRange[1], 320, -0.5, 319.5);
	hPosAlignedDt = new TH2F("hPosAlignedDt","hPosAlignedDt", posBins, posRange[0], posRange[1], 100, 0, 50e3);
	hPosAlignedTotX = new TH2F("hPosAlignedTotX","hPosAlignedTotX", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedTotY = new TH2F("hPosAlignedTotY","hPosAlignedTotY", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedTotU = new TH2F("hPosAlignedTotU","hPosAlignedTotU", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedGateX = new TH2F("hPosAlignedGateX","hPosAlignedGateX", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	hPosAlignedGateY = new TH2F("hPosAlignedGateY","hPosAlignedGateY", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	hPosAlignedGateU = new TH2F("hPosAlignedGateU","hPosAlignedGateU", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	// for chain
	const Int_t rateBins = 200;
	const Float_t rateRange[2] = {0.0e6, 8.0e6};
	hPosAlignedRate = new TH2F("hPosAlignedRate","hPosAlignedRate", posBins, posRange[0], posRange[1], rateBins, rateRange[0], rateRange[1]);
    gRatePosAligned = new TGraph();

	// loop through all inputTree
	Long64_t events = inputTree->GetEntries();
	Long64_t nskipped = 0;
	Long64_t pos = 0;
	Long64_t posAligned = 0;
	Long64_t dt = 0;

	for (Long64_t event = 0; event < events; event++) {
		printProgress(event, events);

		inputTree->GetEntry(event);

		// xiX contains the gate entry so has to be > 1
		if (!(xiVectors[0]->size() > 0 && xiVectors[1]->size() > 0 && xiVectors[2]->size() > 0)) {
			nskipped++;
			continue;
		}
		// if (!((*timeVectors[0])[0] < (*timeVectors[1])[0] && (*timeVectors[1])[0] < (*timeVectors[2])[0])) {
		// 	nskipped++;
		// 	continue;
		// }
		if (!(
			(*totVectors[0])[0] >= totRange[0] && (*totVectors[0])[0] < totRange[1] &&
			(*totVectors[1])[0] >= totRange[0] && (*totVectors[1])[0] < totRange[1] &&
			(*totVectors[2])[0] >= totRange[0] && (*totVectors[2])[0] < totRange[1]
		)) {
			nskipped++;
			continue;
		}
		// if (!(pow(m1q*m2q, 0.5) >= qRange[0] && pow(m1q*m2q, 0.5) < qRange[1])) {
		// 	nskipped++;
		// 	continue;
		// } 
		// if (!(s1q >= qRange[0] && s1q < qRange[1])) {
		// 	nskipped++;
		// 	continue;
		// } 

		pos = (*xiVectors[0])[0] + (*xiVectors[1])[0];
		posAligned = (*xiVectors[0])[0] + (*xiVectors[1])[0] - ((*xiVectors[2])[0] + 39.14285)/0.73142;
		dt = abs((*timeVectors[0])[0] - (*timeVectors[1])[0]) + abs((*timeVectors[1])[0] - (*timeVectors[2])[0]) + abs((*timeVectors[2])[0] - (*timeVectors[0])[0]);

		hPos->Fill(pos, (*xiVectors[2])[0]);
		hPosAligned->Fill(posAligned, (*xiVectors[2])[0]);
		hPosAlignedDt->Fill(posAligned, dt);
		hPosAlignedTotX->Fill(posAligned, (*totVectors[0])[0]);
		hPosAlignedTotY->Fill(posAligned, (*totVectors[1])[0]);
		hPosAlignedTotU->Fill(posAligned, (*totVectors[2])[0]);
		hPosAlignedGateX->Fill(posAligned, (*timeVectors[0])[0] - (*timeGate)[0]);
		hPosAlignedGateY->Fill(posAligned, (*timeVectors[1])[0] - (*timeGate)[0]);
		hPosAlignedGateU->Fill(posAligned, (*timeVectors[2])[0] - (*timeGate)[0]);
		// for chain
		hPosAlignedRate->Fill(posAligned, rate);
	}

	// for chain
	// normalize hPosAlignedRate
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
	// for chain
	hPosAlignedRate->SetTitle(";(xiU+xiY)-(xiU+30.4)/0.7;rate");
	hPosAlignedRateNorm->SetTitle(";(xiU+xiY)-(xiU+30.4)/0.7;rate normalized");

	gRatePosAligned->SetMarkerStyle(20);	// circle marker
	gRatePosAligned->SetMarkerSize(0.5);
	gRatePosAligned->SetMarkerColor(kRed);

	cout << "Skipped " << nskipped << "/" << events << endl;
}
