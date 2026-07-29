#include <iostream>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <array>
#include <limits>

#include <TCanvas.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TMath.h>
#include <TEllipse.h>
#include <TLine.h>
#include <TF1.h>

#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TCanvas *cHits = nullptr;
TH1F *hUDiff;
TEllipse *gTarget = nullptr;
vector<TGraph*> hTotXi(3);
TGraph *gTiming;

vector<Double_t> uIntercept;
vector<Double_t> uSlope;

// coords from togaxsi entrance [mm]
Double_t targetCenterCoord[3] = {-2.5, -4.0, 56.5};
Double_t targetRadius = 17.5;
Double_t fbtCenterCoord[3] = {4.451, 0.208, 889.8}; // target center: 56.5

Double_t xiToPosX(Int_t xiX) {
	return -0.75 * (Double_t)(xiX - 160 - 0.5) + fbtCenterCoord[0];
}
Double_t xiToPosY(Int_t xiY) {
	return -0.75 * (Double_t)(xiY - 112 - 0.5) + fbtCenterCoord[1];
}

struct Track {
	Int_t xiX, xiY, xiU;
	Double_t intercept, slope;
	array<Double_t, 3> coordX, coordY, coordU;
	Double_t uDiff;

	Track(Int_t xiX, Int_t xiY, Int_t xiU, const Double_t (&reactionCoord)[3]) 
		: xiX(xiX), xiY(xiY), xiU(xiU)
	{
		intercept = uIntercept[xiU];
		slope = uSlope[xiU];

		Double_t posX = xiToPosX(xiX);
		Double_t posY = xiToPosY(xiY);
		Double_t reactionDistance = fbtCenterCoord[2] - reactionCoord[2]; // mm
		Double_t tanX = (posX - reactionCoord[0]) / (reactionDistance + LAYER_COORDS[0]); // beam tan for X
		Double_t tanY = (posY - reactionCoord[1]) / (reactionDistance + LAYER_COORDS[1]); // beam tan for X

		coordX = {
			posX, 
			posY + (LAYER_COORDS[0] - LAYER_COORDS[1]) * tanY,
			fbtCenterCoord[2] + LAYER_COORDS[0]
		};
		coordY = {
			posX + (LAYER_COORDS[1] - LAYER_COORDS[0]) * tanX,
			posY,
			fbtCenterCoord[2] + LAYER_COORDS[1]
		};
		coordU = {
			coordY[0] + (LAYER_COORDS[2] - LAYER_COORDS[1]) * tanX,
			coordY[1] + (LAYER_COORDS[2] - LAYER_COORDS[1]) * tanY,
			fbtCenterCoord[2] + LAYER_COORDS[2]
		};

		uDiff = abs(
			(slope * coordU[0] - coordU[1] + intercept)
			/ sqrt(slope * slope + 1.0)
		); // distance between projected hit position and u fiber
	}
};

void printHitsAngleStart(const DataFBT2& inData, const TString& uPosPath) {
	// set branches
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	// initiate objects
	delete cHits;
    cHits = new TCanvas("cHits", "cHits", 10*120, 7*120);
	// for (Int_t i = 0; i < 3; i++) {
	// 	delete hTotXi[i];
	// 	hTotXi[i] = new TGraph();
	// }
	delete hUDiff;
	hUDiff = new TH1F(
		"hUDiff", 
		"hUDiff; u diff [mm]", 
		100, 0, 3
	);
	delete gTarget;
    gTarget = new TEllipse(targetCenterCoord[0], targetCenterCoord[1], targetRadius);

	// // load time offsets from tsv
	// ifstream tsv(uPosPath.Data());
	// string line;
	// // skip header
	// getline(tsv, line);
	// uIntercept.push_back(0);
	// uSlope.push_back(0);
	// while (getline(tsv, line)) {
	// 	istringstream ss(line);
	// 	Int_t xi;
	// 	Double_t intercept;
	// 	Double_t slope;
	// 	ss >> xi >> intercept >> slope;
	//
	// 	uIntercept.push_back(intercept);
	// 	uSlope.push_back(slope);
	// }

	TString graphPath = Form("%s_hitsAngle.pdf", inData.runNum.Data());
	cHits->Print(Form("%s[", graphPath.Data()));
}

int printHitsAngleLoop(
	const DataFBT2& inData, 
	const Double_t (&reactionCoord)[3], 
	UInt_t hits, 
	Bool_t draw
) {
	// check all layers
	for (Int_t layer = 0; layer < 3; layer++) {
		// check if is hit
		if (inData.xiV[layer]->size() < hits) {
			return 0;
		}
		// check tot
		for (UInt_t hit = 0; hit < hits; hit++) {
			if (!inRange((*inData.totV[layer])[hit], {0.5, 10.0})) {
				return 0;
			}
		}
		if (inRange((*inData.totV[layer])[hits], {0.5, 10.0})) {
			return 0;
		}
		// check poisition diff
		// if (abs((*inData.xiV[layer])[1] - (*inData.xiV[layer])[0]) < 5) {
		// 	return 0;
		// }
	}

	// get best fiber combination for each hit
	std::array<Double_t, 2> posBest = {600, 600};
	std::array<std::array<UInt_t, 3>, 2> hitsBest = {};
	for (UInt_t xHit = 0; xHit < hits; xHit++) {
		for (UInt_t yHit = 0; yHit < hits; yHit++) {
			for (UInt_t uHit = 0; uHit < hits; uHit++) {
				Double_t pos = abs(
					(Double_t)((*inData.xiV[0])[xHit] + (*inData.xiV[1])[yHit])
					- (Double_t)((*inData.xiV[2])[uHit] - POS_OFFSET) / POS_SLOPE
				);

				if (pos < posBest[0]) {
					posBest[1] = posBest[0];
					hitsBest[1] = hitsBest[0];

					posBest[0] = pos;
					hitsBest[0] = {xHit, yHit, uHit};
				} else if (pos < posBest[1]) {
					posBest[1] = pos;
					hitsBest[1] = {xHit, yHit, uHit};
				}
			}
		}
	}

	// calculate tracks for each hit
	vector<Track> tracks;
	for (UInt_t hit = 0; hit < hits; hit++) {
		UInt_t xHit = hitsBest[hit][0];
		UInt_t yHit = hitsBest[hit][1];
		UInt_t uHit = hitsBest[hit][2];
		tracks.emplace_back(
			(*inData.xiV[0])[xHit],
			(*inData.xiV[1])[yHit],
			(*inData.xiV[2])[uHit],
			reactionCoord
		);

		hUDiff->Fill(tracks[hit].uDiff);
	}

	// draw and save hits and tracks
	if (!draw) {
		return 0;
	}
	TString graphPath = Form("%s_hitsAngle.pdf", inData.runNum.Data());
	Double_t xRange[2] = {-10*13, 10*13};
	Double_t yRange[2] = {-7*13, 7*13};
	Int_t colors[3] = {kRed, kBlue, kGreen};

	// draw tot graphs
	cHits->Divide(2, 2);
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
	// draw timing graph
	cHits->cd(4);
	gPad->SetGrid();
	delete gTiming;
	gTiming = new TGraph();
	gTiming->SetMarkerStyle(20);
	gTiming->SetMarkerSize(0.5);
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t hit = 0; hit < 2; hit++) {
			gTiming->SetPoint(
				layer * 2 + hit,
				(*inData.timeV[layer])[hit] - (*inData.timeGate)[0],
				layer * 2 + hit
			);
		}
		gTiming->Draw("AP");
		gTiming->SetTitle("hit vs timing;timing [ps];hit");
		gTiming->GetXaxis()->SetLimits(-5000, 25000);
		gTiming->GetYaxis()->SetRangeUser(-0.5, 5.5);
	}
	cHits->Print(graphPath);
	cHits->Clear();

	// draw frame and target
	cHits->Divide(1, 1);
	cHits->cd(1);
	gPad->DrawFrame(xRange[0], yRange[0], xRange[1], yRange[1]);
    gTarget->SetFillStyle(0);
    gTarget->SetLineStyle(1);
    gTarget->SetLineWidth(1);
    gTarget->SetLineColor(kGray);
	gTarget->Draw();

	TGraph gHits;
	gHits.SetPoint(0, reactionCoord[0], reactionCoord[1]);
	for (UInt_t hit = 0; hit < hits; hit++) {
		// draw hits with the biggest tots
		// X fiber
		Double_t posX = xiToPosX((*inData.xiV[0])[hit]);
		TLine xFiber(posX, yRange[0], posX, yRange[1]);
		xFiber.SetLineColor(colors[hit]);
		xFiber.SetLineWidth(1);
		// Y fiber
		Double_t posY = xiToPosY((*inData.xiV[1])[hit]);
		TLine yFiber(xRange[0], posY, xRange[1], posY);
		yFiber.SetLineColor(colors[hit]);
		yFiber.SetLineWidth(1);
		// U fiber
		TF1 uFiber(Form("uFiber_%u", hit),
			"[0]+[1]*x",
			xRange[0], xRange[1]
		);		
		// analytic values
		uFiber.SetParameters(
			(((*inData.xiV[2])[hit] - 160 - 0.5) * (-0.75) + U_OFFSET) * sqrt(2),
			-1
		);
		// empirical values
		// uFiber.SetParameters(
		// 	uIntercept[(*inData.xiV[2])[hit]],
		// 	uSlope[(*inData.xiV[2])[hit]]	
		// );
		uFiber.SetLineColor(colors[hit]);
		uFiber.SetLineWidth(1);

		// set tracks
		gHits.SetPoint(hit * 3 + 1, tracks[hit].coordX[0], tracks[hit].coordX[1]);
		gHits.SetPoint(hit * 3 + 2, tracks[hit].coordY[0], tracks[hit].coordY[1]);
		gHits.SetPoint(hit * 3 + 3, tracks[hit].coordU[0], tracks[hit].coordU[1]);
		// track
		TLine track(reactionCoord[0], reactionCoord[1], tracks[hit].coordU[0], tracks[hit].coordU[1]);
		track.SetLineColor(kGray);
		track.SetLineStyle(3);

		xFiber.DrawClone("SAME");
		yFiber.DrawClone("SAME");
		uFiber.DrawClone("SAME");
		track.DrawClone("SAME");
	}
	gHits.SetMarkerStyle(20);
	gHits.SetMarkerSize(0.3);
	gHits.SetTitle("; x [mm]; y[mm]");
	gHits.DrawClone("P SAME"); // redraw points on top
	// save graph
	// cHits->Modified();
	// cHits->Update();
	cHits->Print(graphPath);
	cHits->Clear();

	return 1;
}

void printHitsAngleEnd(const DataBase& inData) {
	cHits->SetGrid();
	hUDiff->Draw();

	TString graphPath = Form("%s_hitsAngle.pdf", inData.runNum.Data());
	cHits->Print(graphPath);
	cHits->Print(Form("%s]", graphPath.Data()));
}

void printHitsAngle(const vector<TString>& inPaths, const TString& uPosPath, UInt_t hits = 1, Long64_t loopEvents = 100) {
	// load data
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printHitsAngleStart(inData, uPosPath);

	// only draw loopEvents of events
	Long64_t drawnEvents = 0;
	if (loopEvents == 0) {
		loopEvents = inData.entries;
	}

	Double_t reactionCoord[3] = {0, 0, targetCenterCoord[2]};
	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		if (drawnEvents < loopEvents) {
			drawnEvents += printHitsAngleLoop(inData, reactionCoord, hits, 1);
		} else {
			printHitsAngleLoop(inData, reactionCoord, hits, 0);
		}
	}

	printHitsAngleEnd(inData);
}

