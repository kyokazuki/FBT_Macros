#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>

#include <TCanvas.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TMath.h>
#include <TPolyLine.h>

#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TCanvas *cHits = nullptr;
TH2F *hHits = new TH2F(
	"hHits",
	"hit fiber positions;xiX;xiY",
	320, 0.5, 320.5,
	224, 0.5, 224.5
);
vector<TGraph*> hTotXi(3);
TGraph *hTiming;

void drawHit(
	Int_t xi,
	Int_t layer,
	Int_t hit,
	Float_t tot
) {
	// only use the layer element
	// Double_t layerCenterX[3] = {(Double_t)xi, 160.5, ((Double_t)xi + 39.14285) / 1.41421};
	// Double_t layerCenterY[3] = {112.5, (Double_t)xi, ((Double_t)xi + 39.14285) / 1.41421};
	Double_t layerCenterX[3] = {(Double_t)xi, 160.5, ((Double_t)xi + 33) / 1.41421};
	Double_t layerCenterY[3] = {112.5, (Double_t)xi, ((Double_t)xi + 33) / 1.41421};
	Double_t layerWidths[3] = {1, 1, 1.4};
	Double_t layerHeights[3] = {224, 320, 224};
	Double_t layerAngles[3] = {0, 90, 45};

    // local coordinates from center
	Double_t width = layerWidths[layer];
	Double_t height = layerHeights[layer];
    Double_t dx[4] = {-width/2.,  width/2.,  width/2., -width/2.};
    Double_t dy[4] = {-height/2., -height/2., height/2., height/2.};

	// global coordinates
	Double_t centerX = layerCenterX[layer];
	Double_t centerY = layerCenterY[layer];
    Double_t theta = layerAngles[layer] * TMath::DegToRad();
    Double_t x[5], y[5];
    for (Int_t i = 0; i < 4; i++) {
        x[i] = centerX + dx[i]*cos(theta) - dy[i]*sin(theta);
        y[i] = centerY + dx[i]*sin(theta) + dy[i]*cos(theta);
    }
    x[4] = x[0];
    y[4] = y[0];

	// draw ploygon
	TPolyLine poly(5, x, y);
	poly.SetLineColor(0);
	if (hit == 0) {
		poly.SetFillColorAlpha(kRed, 1);
	} else if (hit == 1) {
		poly.SetFillColorAlpha(kBlue, 1);
	} else if (hit == 2) {
		poly.SetFillColorAlpha(kGreen, 1);
	// } else {
	// 	tot = TMath::Min((Double_t)tot, 1.0);
	// 	Int_t darkness = (Int_t)TMath::Floor((1 - tot) * 255);
	// 	Int_t color = TColor::GetColor(darkness, darkness, darkness);
	// 	poly.SetFillColorAlpha(color, 0.8);
	}
	poly.DrawClone("f same");
}

void printHitsRecStart(const DataFBT2& inData) {
	// set branches
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete cHits;
    cHits = new TCanvas("cHits", "cHits", 30*40, 14*40);
	// gStyle->SetOptStat(0);

	TString graphPath = Form("%s_hitsRec.pdf", inData.runNum.Data());
	cHits->Print(Form("%s[", graphPath.Data()));
}

int printHitsRecLoop(const DataFBT2& inData) {
	// check all layers
	for (Int_t layer = 0; layer < 3; layer++) {
		// check if is hit
		if (inData.xiV[layer]->size() < 2) {
			return 0;
		}
		// check tot
		if (!inRange((Double_t)(*inData.totV[layer])[0], {5e-1, 1e1})) {
			return 0;
		}
		// check tot
		if (!inRange((Double_t)(*inData.totV[layer])[1], {5e-1, 1e1})) {
			return 0;
		}
		// check poisition diff
		if (abs((*inData.xiV[layer])[1] - (*inData.xiV[layer])[0]) < 5) {
			return 0;
		}
	}

	cHits->Divide(3, 2);

	for (Int_t layer = 0; layer < 3; layer++) {
		cHits->cd(layer + 1);
		gPad->SetGrid();

		delete hTotXi[layer];
		hTotXi[layer] = new TGraph();
		hTotXi[layer]->SetMarkerStyle(20);
		hTotXi[layer]->SetMarkerSize(0.5);
		for (UInt_t hit = 0; hit < inData.xiV[layer]->size(); hit++) {
			hTotXi[layer]->SetPoint(hit, (*inData.xiV[layer])[hit], (*inData.totV[layer])[hit]);
		}
		hTotXi[layer]->Draw("AP");
		hTotXi[layer]->SetTitle(Form("tot vs xi%c;xi%c;tot", LAYER_NAMES[layer], LAYER_NAMES[layer]));
		hTotXi[layer]->GetXaxis()->SetLimits(0.5, LAYER_CHANNELS[layer] + 0.5);
		hTotXi[layer]->GetYaxis()->SetRangeUser(0, 5);
	}

	cHits->cd(4);
	gPad->SetGrid();
	delete hTiming;
	hTiming = new TGraph();
	hTiming->SetMarkerStyle(20);
	hTiming->SetMarkerSize(0.5);
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < 2; hit++) {
			hTiming->SetPoint(
				layer * 2 + hit,
				(*inData.timeV[layer])[hit] - (*inData.timeGate)[0],
				layer * 2 + hit
			);
		}
		hTiming->Draw("AP");
		hTiming->SetTitle("hit vs timing;timing [ps];hit");
		hTiming->GetXaxis()->SetLimits(-5000, 25000);
		hTiming->GetYaxis()->SetRangeUser(-0.5, 5.5);
	}

	cHits->cd(5);
	gPad->SetGrid();
	hHits->SetStats(0);
	hHits->Draw();
	for (Int_t layer = 0; layer < 3; layer++) {
		// for (UInt_t hit = 0; hit < inData.xiV[layer]->size(); hit++) {
		for (UInt_t hit = 0; hit < 2; hit++) {
			// if ((*inData.totV[layer])[hit] < 0.5) {
			// 	continue;
			// }

			drawHit(
				(*inData.xiV[layer])[hit], 
				layer, 
				hit,
				(*inData.totV[layer])[hit]
			);
		}
	}

	// save graph
    cHits->Update();
	TString graphPath = Form("%s_hitsRec.pdf", inData.runNum[0].Data());
	cHits->Print(graphPath);
	cHits->Clear();

	return 1;
}

void printHitsRecEnd(const DataBase& inData) {
	TString graphPath = Form("%s_hitsRec.pdf", inData.runNum[0].Data());
	cHits->Print(Form("%s]", graphPath.Data()));
	delete cHits;
}

void printHitsRec(const TString& inPath, Long64_t loopEvents = 0) {
	// load data
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	printHitsRecStart(inData);

	// only draw loopEvents of events
	Long64_t drawnEvents = 0;
	if (loopEvents == 0) {
		loopEvents = inData.entries;
	}

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		drawnEvents += printHitsRecLoop(inData);
		if (drawnEvents == loopEvents) {
			break;
		}
	}

	printHitsRecEnd(inData);
}

