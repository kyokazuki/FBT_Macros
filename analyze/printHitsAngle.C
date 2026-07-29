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

#include "utils/addStats.C"
#include "utils/constants.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

TCanvas *cHits = nullptr;
TH1F *hUDiff;
TEllipse *gTarget = nullptr;
vector<TGraph*> hTotXi(3);
TGraph *gTiming;

// coords from togaxsi entrance [mm]
Double_t targetCenterCoord[3] = {-2.5, -4.0, 56.5};
Double_t targetRadius = 17.5;
Double_t fbtCenterCoord[3] = {4.451, 0.208, 889.8}; // target center: 56.5
Double_t uOffset = U_OFFSET + (fbtCenterCoord[0] + fbtCenterCoord[1]) / sqrt(2);

Double_t xiToPosX(Int_t xiX) {
	return -0.75 * (Double_t)(xiX - 160 - 0.5) + fbtCenterCoord[0];
}
Double_t xiToPosY(Int_t xiY) {
	return -0.75 * (Double_t)(xiY - 112 - 0.5) + fbtCenterCoord[1];
}

struct Track {
	Int_t xiX, xiY, xiU;
	Double_t intercept, slope, uDiff;
	array<Double_t, 3> coordX, coordY, coordU;

	Track(Int_t xiX, Int_t xiY, Int_t xiU, const Double_t (&reactionCoord)[3]) 
		: xiX(xiX), xiY(xiY), xiU(xiU)
	{
		intercept = ((xiU - 160 - 0.5) * (-0.75) + uOffset) * sqrt(2);
		slope = -1.0;

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

		// distance between projected hit position and u fiber
		// uDiff = abs(
		// 	(slope * coordU[0] - coordU[1] + intercept)
		// 	/ sqrt(slope * slope + 1.0)
		// );
		uDiff = (slope * coordU[0] - coordU[1] + intercept) / sqrt(slope * slope + 1.0); 
	}
};

void printHitsAngleStart(
	const DataFBT2& inData,
	const TString& outName
) {
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
	for (Int_t i = 0; i < 3; i++) {
		delete hTotXi[i];
		hTotXi[i] = new TGraph();
	}
	delete hUDiff;
	hUDiff = new TH1F(
		"hUDiff", 
		"hUDiff; u diff [mm]", 
		200, -3, 3
	);
	delete gTarget;
    gTarget = new TEllipse(targetCenterCoord[0], targetCenterCoord[1], targetRadius);

	cHits->Print(Form("%s[", outName.Data()));
}

int printHitsAngleLoop(
	const DataFBT2& inData, 
	const Double_t (&reactionCoord)[3], 
	UInt_t hits, 
	const TString& outName,
	Bool_t saveGraph
) {
	vector<vector<UInt_t>> validHits(3);
	for (Int_t layer = 0; layer < 3; layer++) {
		// differentiate hits
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			// ToT >= 0.5 necessary
			if ((*inData.totV[layer])[hit] < 0.5) {
				break;
			}

			// first hit always valid
			if (hit == 0) {
				validHits[layer].push_back(hit);
				if (validHits[layer].size() == hits) {
					break;
				} else {
					continue;
				}
			}

			// valid if position diff >= 5
			if (abs((*inData.xiV[layer])[hit] - (*inData.xiV[layer])[validHits[layer][0]]) >= 5) {
				validHits[layer].push_back(hit);
				if (validHits[layer].size() == hits) {
					break;
				} else {
					continue;
				}
			}
			// valid if time diff >= 1000 ps
			if (abs((*inData.timeV[layer])[hit] - (*inData.xiV[layer])[validHits[layer][0]]) < 1000) {
				validHits[layer].push_back(hit);
				if (validHits[layer].size() == hits) {
					break;
				} else {
					continue;
				}
			}
		}

		// skip event if not enough valid hits
		if (validHits[layer].size() < hits) {
			return 0;
		}
	}

	// get best fiber combination for each hit
	std::array<Double_t, 2> posBest = {600, 600};
	std::array<std::array<UInt_t, 3>, 2> hitsBest = {};
	for (const UInt_t& validHitX : validHits[0]) {
		for (const UInt_t& validHitY : validHits[1]) {
			for (const UInt_t& validHitU : validHits[2]) {
				Double_t pos = abs(
					(Double_t)((*inData.xiV[0])[validHitX] + (*inData.xiV[1])[validHitY])
					- (Double_t)((*inData.xiV[2])[validHitU] - POS_OFFSET) / POS_SLOPE
				);

				if (pos < posBest[0]) {
					posBest[1] = posBest[0];
					hitsBest[1] = hitsBest[0];

					posBest[0] = pos;
					hitsBest[0] = {validHitX, validHitY, validHitU};
				} else if (pos < posBest[1]) {
					posBest[1] = pos;
					hitsBest[1] = {validHitX, validHitY, validHitU};
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
	if (!saveGraph) {
		return 0;
	}
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
		for (UInt_t hit = 0; hit < hits; hit++) {
			gTiming->SetPoint(
				layer * 2 + hit,
				(*inData.timeV[layer])[hit] - (*inData.timeGate)[0],
				layer * 2 + hit
			);
		}
	}
	gTiming->Draw("AP");
	gTiming->SetTitle("hit vs timing;timing [ps];hit");
	gTiming->GetXaxis()->SetLimits(-5000, 15000);
	gTiming->GetYaxis()->SetRangeUser(-0.5, 5.5);
	cHits->Print(outName);
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
			(((*inData.xiV[2])[hit] - 160 - 0.5) * (-0.75) + uOffset) * sqrt(2),
			-1
		);
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
	gHits.SetMarkerSize(0.4);
	gHits.SetTitle("; x [mm]; y[mm]");
	gHits.DrawClone("P SAME"); // redraw points on top
	// save graph
	cHits->Print(outName);
	cHits->Clear();

	return 1;
}

void printHitsAngleEnd(
	const DataBase& inData,
	const TString& outName
) {
	hUDiff->Fit("gaus");
	TF1 *fit = hUDiff->GetFunction("gaus");
	addStats(hUDiff, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hUDiff->GetEntries()), 
		Form("amp = %.4g", fit->GetParameter(0)),
		Form("mean = %.4g", fit->GetParameter(1)),
		Form("sigma = %.4g", fit->GetParameter(2))
	});

	cHits->SetGrid();
	hUDiff->Draw();

	cHits->Print(outName);
	cHits->Print(Form("%s]", outName.Data()));
}

void printHitsAngle(
	const vector<TString>& inPaths,
	UInt_t hits = 1,
	Long64_t loopEvents = 100
) {
	// load data
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	TString outName = Form("%s_hitsAngle_%dhits.pdf", inData.runNum.Data(), hits);
	printHitsAngleStart(inData, outName);

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
			drawnEvents += printHitsAngleLoop(inData, reactionCoord, hits, outName, 1);
		} else {
			printHitsAngleLoop(inData, reactionCoord, hits, outName, 0);
		}
	}

	printHitsAngleEnd(inData, outName);
}

