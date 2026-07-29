#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <iomanip>
#include <array>

#include <TTree.h>
#include <TString.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TH1F.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TMath.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"

vector<TGraph*> gUPos(320, nullptr);
TGraph* gUGap = nullptr;

Double_t fbtCenterCoord[3] = {4.088, -0.174, 920 + 56.5}; // target center: 56.5

void printUPosStart(const DataFBT2& inData) {
	// set branches
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
	}

	// initiate histograms
	for (Int_t i = 0; i < 320; i++) {
		delete gUPos[i];
		gUPos[i] = new TGraph();
	}
	delete gUGap;
	gUGap = new TGraph();
}

void printUPosLoop(const DataFBT2& inData, Double_t targetDistance, Bool_t adjustAngle) {
	// check all layers
	for (Int_t layer = 0; layer < 3; layer++) {
		// check if is hit
		if (inData.xiV[layer]->size() == 0) {
			return;
		}
		// check tot
		if (!inRange((*inData.totV[layer])[0], {0.5, 10.0})) {
			return;
		}
		// remove multi hits
		if ((*inData.totV[layer])[1] > 0.5) {
			return;
		}
	}
	// filter out irrelevant hits
	if (!inRange(((*inData.xiV[0])[0] + (*inData.xiV[1])[0]) - ((*inData.xiV[2])[0] - POS_OFFSET) / POS_SLOPE, {-10, 10})) {
		return;
	}

	// Double_t reactionCoord[3] = {-2.5, -4.0, 56.5}; // = targetCenterCoord
	Double_t reactionCoord[3] = {0.0, 0.0, 56.5}; // = targetCenterCoord
	Double_t posX = -0.75 * (Double_t)((*inData.xiV[0])[0] - 160) + 4.088; // convert to mm
	Double_t posY = -0.75 * (Double_t)((*inData.xiV[1])[0] - 112) + (-0.174); // convert to mm

	// adjust for angle from target center
	if (adjustAngle) {
		posX += (posX - reactionCoord[0]) / (targetDistance + LAYER_COORDS[0]) * (LAYER_COORDS[2] - LAYER_COORDS[0]);
		posY += (posY - reactionCoord[1]) / (targetDistance + LAYER_COORDS[1]) * (LAYER_COORDS[2] - LAYER_COORDS[1]);
	}

	Int_t points = gUPos[(*inData.xiV[2])[0] - 1]->GetN();
	if (points < 10000) {
		gUPos[(*inData.xiV[2])[0] - 1]->SetPoint(points, posX, posY);
	}
}

void printUPosEnd(
	const DataBase& inData, 
	Double_t targetDistance, 
	Int_t gapPitch, 
	const TString& outName
) {
	// output fitted U fiber position to tsv
	ofstream out(Form("%s.tsv", outName.Data()));
	out << "xi\tintercept\tslope" << endl;
	out << fixed << setprecision(3);

	// fit and print the graphs for each U
	TCanvas *cUPos = new TCanvas("cUPos", "cUPos", 700, 500);
	TString graphPath = Form("%s.pdf", outName.Data());
	cUPos->Print(Form("%s[", graphPath.Data()));
	array<array<Double_t, 2>, 320 - 1> crosspoints;
	for (Int_t i = 0; i < 320; i++) {
		cUPos->Clear();

		cUPos->SetGrid();
		gUPos[i]->Draw("APX");
		gUPos[i]->SetTitle(Form("%d events;x [mm];y [mm]", gUPos[i]->GetN()));
		gUPos[i]->GetXaxis()->SetLimits(-126, 126);
		gUPos[i]->GetYaxis()->SetRangeUser(-90, 90);
		gUPos[i]->Fit("pol1", "Q");
		TF1 *fit = gUPos[i]->GetFunction("pol1");
		fit->SetLineWidth(1);
		Double_t intercept = fit->GetParameter(0);
		Double_t slope = fit->GetParameter(1);
		out << i + 1 << "\t" << intercept << "\t" << slope << endl;
		cUPos->Print(graphPath);

		// calculate crosspoint with y=(7/10)*x to evaulate fiber gaps
		if (i == 319) {
			break;
		}
		crosspoints[i][0] = intercept / (0.7 - slope);
		crosspoints[i][1] = 0.7 * crosspoints[i][0];
		if (i >= gapPitch) {
			Double_t gap = sqrt(
				TMath::Power(crosspoints[i][0] - crosspoints[i - gapPitch][0], 2)
				+ TMath::Power(crosspoints[i][1] - crosspoints[i - gapPitch][1], 2)
			);
			gUGap->SetPoint(gUGap->GetN(), (Double_t)i - (Double_t)gapPitch / 2, gap);
		}
	}

	// print the fits for all U
	cUPos->Clear();
	cUPos->DrawFrame(
		-125, -90, 125, 90, 
		Form("target-FBT distance = %.1f [mm];x [mm];y [mm]", targetDistance)
	);
	for (auto *g : gUPos) {
		if (auto *f = g->GetFunction("pol1"))
			f->Draw("SAME");
	}
	cUPos->Print(graphPath);
	
	// print gap graph
	cUPos->Clear();
	gUGap->Draw("APX");
	gUGap->SetTitle(Form("u fiber gaps [mm] (gap pitch: %d);xiU; gap [mm]", gapPitch));
	gUGap->GetXaxis()->SetLimits(1, 320);
	gUGap->GetYaxis()->SetRangeUser(-0.5, 3.5);
	cUPos->Print(graphPath);

	cUPos->Print(Form("%s]", graphPath.Data()));
}

void printUPos(const vector<TString>& inPaths, Double_t targetDistance, Int_t gapPitch = 1, Bool_t adjustAngle = 1) {
	// load data
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printUPosStart(inData);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printUPosLoop(inData, targetDistance, adjustAngle);
	}

	TString outName;
	if (adjustAngle) {
		outName = Form("%s_uPos", inData.runNum.Data());
	} else {
		outName = Form("%s_uPos_unadjusted", inData.runNum.Data());
	}
	printUPosEnd(inData, targetDistance, gapPitch, outName);
}

