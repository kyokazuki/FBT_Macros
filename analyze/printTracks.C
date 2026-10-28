#include <iostream>
#include <stdlib.h>
#include <vector>
#include <array>
#include <algorithm>

#include <TCanvas.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TStyle.h>
#include <TMath.h>
#include <TEllipse.h>
#include <TBox.h>
#include <TLine.h>
#include <TF1.h>
#include <TText.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/track.C"

// x and y range
Double_t xPosRange[2] = {xiToPosGlobal(320, 0), xiToPosGlobal(1, 0)};
Double_t yPosRange[2] = {xiToPosGlobal(224, 1), xiToPosGlobal(1, 1)};

// canvas
TCanvas *cHits = nullptr;
// tot vs xi
array<TGraph*, 3> hTotXi;
// timing of each cluster
TGraph *gTiming;
// target
TEllipse *gTarget = nullptr;
// togaxsi window
TBox *gWindow = nullptr;
// fiber and window hits
TGraph *gHits;

// track count and type graph
TH1F* hTracks;
array<Long64_t, 4> threeLayerTracksEvents; // count for 1~4 3LT-only events
// propagation time
array<TH2F*, 4> hProp; // for XYU and all combined
// projected position and real position diff
array<TH1F*, 3> hProjectionDiff;
// time diff between layers for 3LTs
array<TH1F*, 3> hTimeDiff;
// time std dev for 3LT and 2LT
TH1F* hTimeSD3;
TH1F* hTimeSD2;
// 3LT timeSD and projectionDiff correlation
array<TH2F*, 3> hTimeSDProjectionDiff;
array<TGraph*, 3> gTimeSDProjectionDiffSigma;
// tot std dev for 3LT and 2LT
TH1F* hTotSD3;
TH1F* hTotSD2;
// 3LT totSD and projectionDiff correlation
array<TH2F*, 3> hTotSDProjectionDiff;
array<TGraph*, 3> gTotSDProjectionDiffSigma;
// timeSD vs totSD
TGraph* gTotSDTimeSD;
// distances between 3LTs
array<TH1F*, 3> hTrackDistance;

void printTracksStart(
	const DataFBTCoin& inData,
	const TString& outName
) {
	// set branches
	inData.tree->SetBranchStatus("coin", 1);
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("time%c", LAYER_NAMES[layer]), 1);
	}

	delete cHits;
    cHits = new TCanvas("cHits", "cHits", 10*120, 7*120);

	delete hTracks;
	hTracks = new TH1F(
		"hTracks",
		"hTracks; track count", 
		5, -0.5, 4.5
	);

	delete hProp[3];
	hProp[3] = new TH2F(
		"hPropAll",
		"timing vs distance;distance [mm];timing [ps]",
		320, -159.5 * 0.75, 160.5 * 0.75, 
		200, -5000, 5000
	);

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hTotXi[layer];
		hTotXi[layer] = new TGraph();

		delete hProp[layer];
		hProp[layer] = new TH2F(
			Form("hProp%c", LAYER_NAMES[layer]),
			Form("timing vs distance (%c);distance [mm];timing [ps]", LAYER_NAMES[layer]),
			320, -159.5 * 0.75, 160.5 * 0.75, 
			200, -5000, 5000
		);

		delete hProjectionDiff[layer];
		hProjectionDiff[layer] = new TH1F(
			Form("hProjectionDiff%c", LAYER_NAMES[layer]),
			Form("hProjectionDiff%c; residual [mm]", LAYER_NAMES[layer]), 
			200, -3, 3
		);

		delete hTimeDiff[layer];
		hTimeDiff[layer] = new TH1F(
			Form("hTimeDiff%c", LAYER_NAMES[layer]),
			Form("hTimeDiff (%c-%c); residual [ps]", LAYER_NAMES[(layer + 1) % 3], LAYER_NAMES[(layer + 2) % 3]), 
			200, -1500, 1500
		);

		delete hTrackDistance[layer];
		hTrackDistance[layer] = new TH1F(
			Form("hTrackDistance%c", LAYER_NAMES[layer]),
			Form("hTrackDistance%c; residual [mm]", LAYER_NAMES[layer]), 
			200, 0, 200
		);

		delete hTimeSDProjectionDiff[layer];
		hTimeSDProjectionDiff[layer] = new TH2F(
			Form("hTimeSDProjectionDiff%c", LAYER_NAMES[layer]),
			Form("ProjectionDiff%c vs TimeSD; time std dev [ps]; projectionDiff [mm]", LAYER_NAMES[layer]), 
			200, 0, 1000,
			200, -3, 3
		);
		delete gTimeSDProjectionDiffSigma[layer];
		gTimeSDProjectionDiffSigma[layer] = new TGraph();

		delete hTotSDProjectionDiff[layer];
		hTotSDProjectionDiff[layer] = new TH2F(
			Form("hTotSDProjectionDiff%c", LAYER_NAMES[layer]),
			Form("ProjectionDiff%c vs TotSD; tot std dev [ps]; projectionDiff [mm]", LAYER_NAMES[layer]), 
			200, 0, 1,
			200, -3, 3
		);
		delete gTotSDProjectionDiffSigma[layer];
		gTotSDProjectionDiffSigma[layer] = new TGraph();
	}

	delete hTimeSD3;
	hTimeSD3 = new TH1F(
		"hTimeSD3",
		"hTimeSD3; time std dev [ps]", 
		200, 0, 1000
	);
	delete hTimeSD2;
	hTimeSD2 = new TH1F(
		"hTimeSD2",
		"hTimeSD2; time std dev [ps]", 
		200, 0, 1000
	);

	delete hTotSD3;
	hTotSD3 = new TH1F(
		"hTotSD3",
		"hTotSD3; tot std dev", 
		200, 0, 1
	);
	delete hTotSD2;
	hTotSD2 = new TH1F(
		"hTotSD2",
		"hTotSD2; tot std dev", 
		200, 0, 1
	);

	// initiate objects
	delete gTarget;
    gTarget = new TEllipse(TARGET_CENTER[0], TARGET_CENTER[1], TARGET_RADIUS);
	delete gWindow;
	gWindow = new TBox(35, 35, -35, -35);

	cHits->Print(Form("%s[", outName.Data()));
}

Int_t printTracksLoop(
	const DataFBTCoin& inData, 
	const Double_t (&reactionCoord)[3], 
	Int_t clusterSize,
	Bool_t windowed,
	Bool_t propagated,
	const TString& outName,
	Bool_t saveGraph
) {
	// select coin
	// if (!(
	// 	inData.coin[0]==0 && (inData.coin[1]==1 || inData.coin[2]==1 || inData.coin[3]==1 || inData.coin[4]==1 || inData.coin[5]==1) && inData.coin[6]==0
	// 	// inData.coin[0]==1 && inData.coin[1]==0 && inData.coin[2]==0 && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0
	// )) {
	// 	return 0;
	// }

	// count hits in layers
	array<vector<UInt_t>, 3> availableHits;
	array<vector<vector<UInt_t>>, 3> clusters;
	array<UInt_t, 3> layerHits = {0, 0, 0}; // hit counts for layer N
	Int_t noHitsLayers = 0;
	UInt_t maxHits = 0;
	Int_t maxHitsLayer = 2;
	for (Int_t layer = 0; layer < 3; layer++) {
		for (UInt_t entry = 0; entry < inData.totV[layer]->size(); entry++) {
			// tot condition
			if ((*inData.totV[layer])[entry] < 0.5) {
				break;
			}

			Bool_t isCluster = 0;
			for (UInt_t i = 0; i < availableHits[layer].size(); i++) {
				Int_t xiDiff = abs((*inData.xiV[layer])[entry] - (*inData.xiV[layer])[availableHits[layer][i]]);
				if (xiDiff <= clusterSize) {
					isCluster = 1;
					clusters[layer][i].push_back(entry);
					break;
				}
			}
			if (!isCluster) {
				clusters[layer].push_back({entry});
				availableHits[layer].push_back(entry);
				layerHits[layer]++;
			}
		}

		if (availableHits[layer].size() == 0) {
			noHitsLayers++;
		}

		if (availableHits[layer].size() > maxHits) {
			maxHits = availableHits[layer].size();
			maxHitsLayer = layer;
		}
	}
	
	// set maxHitsLayer to U if possible
	// if (layerHits[0] == maxHits) {
	// 	maxHitsLayer = 0;
	// }
	// if (layerHits[1] == maxHits) {
	// 	maxHitsLayer = 1;
	// }
	if (layerHits[2] == maxHits) {
		maxHitsLayer = 2;
	}

	// discard if not enough hits
	if (noHitsLayers >= 2) {
		return 0;
	}

	// extract 3-fiber tracks
	vector<Track> threeLayerTracks;
	if (noHitsLayers == 0) {
		Int_t layer1 = (maxHitsLayer + 1) % 3;
		Int_t layer2 = (maxHitsLayer + 2) % 3;

		vector<Track> trackCombs;
		for (UInt_t hit1 : availableHits[layer1]) {
			for (UInt_t hit2 : availableHits[layer2]) {
				Track track(
					maxHitsLayer,
					hit1,
					hit2,
					(*inData.xiV[layer1])[hit1],
					(*inData.xiV[layer2])[hit2],
					reactionCoord
				);
				// add the last layer
				for (UInt_t hit3 : availableHits[maxHitsLayer]) {
					track.getProjectionDiff(
						hit3,
						(*inData.xiV[maxHitsLayer])[hit3]
					);
					trackCombs.push_back(track);
				}
			}
		}
		// Best geometrical matches first
		sort(
			trackCombs.begin(),
			trackCombs.end(),
			[](const Track& a, const Track& b) {
				return abs(a.projectionDiff) < abs(b.projectionDiff);
			}
		);

		// Select tracks, making sure no fiber hit is used twice
		for (Track& track : trackCombs) {
			// Only accept sufficiently good matches
			if (abs(track.projectionDiff) > 10) {
				break;
			}
			// check if is within TOGAXSI window
			if (!(windowed == 0 || (windowed && track.inWindow))) {
				continue;
			}

			// Check that these hits are still available
			bool hit1Available =
				find(
					availableHits[layer1].begin(),
					availableHits[layer1].end(),
					track.hit[layer1]
				) != availableHits[layer1].end();
			bool hit2Available =
				find(
					availableHits[layer2].begin(),
					availableHits[layer2].end(),
					track.hit[layer2]
				) != availableHits[layer2].end();
			bool hit3Available =
				find(
					availableHits[maxHitsLayer].begin(),
					availableHits[maxHitsLayer].end(),
					track.hit[maxHitsLayer]
				) != availableHits[maxHitsLayer].end();
			if (!hit1Available || !hit2Available || !hit3Available) {
				continue;
			}

			// Calculate timeSD then accept this track
			track.getTimeSD(
				(*inData.timeV[layer1])[track.hit[layer1]],
				(*inData.timeV[layer2])[track.hit[layer2]],
				(*inData.timeV[maxHitsLayer])[track.hit[maxHitsLayer]],
				propagated
			);
			track.getTotSD(
				(*inData.totV[layer1])[track.hit[layer1]],
				(*inData.totV[layer2])[track.hit[layer2]],
				(*inData.totV[maxHitsLayer])[track.hit[maxHitsLayer]]
			);
			threeLayerTracks.push_back(track);

			// Remove the three used hits
			availableHits[layer1].erase(
				remove(
					availableHits[layer1].begin(),
					availableHits[layer1].end(),
					track.hit[layer1]
				),
				availableHits[layer1].end()
			);
			availableHits[layer2].erase(
				remove(
					availableHits[layer2].begin(),
					availableHits[layer2].end(),
					track.hit[layer2]
				),
				availableHits[layer2].end()
			);
			availableHits[maxHitsLayer].erase(
				remove(
					availableHits[maxHitsLayer].begin(),
					availableHits[maxHitsLayer].end(),
					track.hit[maxHitsLayer]
				),
				availableHits[maxHitsLayer].end()
			);
		}

		// Recalculate number of empty layers
		noHitsLayers = 0;
		for (Int_t layer = 0; layer < 3; layer++) {
			if (availableHits[layer].empty()) {
				noHitsLayers++;
			}
		}
	}

	// extract 2-fiber tracks if only two fiber remains
	vector<Track> twoLayerTracks;
	Int_t noHitsLayer = -1;
	if (noHitsLayers == 1 && availableHits[0].size() + availableHits[1].size() + availableHits[2].size() == 2) {
		if (availableHits[0].size() == 0) {
			noHitsLayer = 0;
		} else if (availableHits[1].size() == 0) {
			noHitsLayer = 1;
		} else if (availableHits[2].size() == 0) {
			noHitsLayer = 2;
		}
		Int_t layer1 = (noHitsLayer + 1) % 3;
		Int_t layer2 = (noHitsLayer + 2) % 3;

		Track track(
			noHitsLayer,
			availableHits[layer1][0],
			availableHits[layer2][0],
			(*inData.xiV[layer1])[availableHits[layer1][0]],
			(*inData.xiV[layer2])[availableHits[layer2][0]],
			reactionCoord
		);
		// Check that track is within range
		if (inRange(track.coord[0][0], xPosRange) && inRange(track.coord[1][1], yPosRange)) {
			// check if is within TOGAXSI window
			if (windowed == 0 || (windowed && track.inWindow)) {
				track.getTimeSD(
					(*inData.timeV[layer1])[availableHits[layer1][0]],
					(*inData.timeV[layer2])[availableHits[layer2][0]],
					propagated
				);
				track.getTotSD(
					(*inData.totV[layer1])[availableHits[layer1][0]],
					(*inData.totV[layer2])[availableHits[layer2][0]]
				);
				twoLayerTracks.push_back(track);
			}
		}
	}

	// plot to hTracks
	Int_t totalTracks = threeLayerTracks.size() + twoLayerTracks.size();
	hTracks->Fill(totalTracks);
	if (twoLayerTracks.size() == 0 && totalTracks >= 1 && totalTracks <= 4) {
		threeLayerTracksEvents[totalTracks - 1]++;
	}
	// plot propagation and projectionDiff and timeSD
	for (Track track : threeLayerTracks) {
		for (Int_t layer = 0; layer < 3; layer++) {
			Double_t distance = getDistanceGlobal(track.coord, track.xi, layer);
			hProp[layer]->Fill(
				distance,
				track.time[layer] - (*inData.timeGate)[0]
			);
			// only fill X and Y into hPropAll
			// if (layer == 2) {
			// 	break;
			// }
			hProp[3]->Fill(
				distance,
				track.time[layer] - (*inData.timeGate)[0]
			);
		}

		hProjectionDiff[maxHitsLayer]->Fill(track.projectionDiff);
		hTimeSD3->Fill(track.timeSD);
		hTimeSDProjectionDiff[maxHitsLayer]->Fill(track.timeSD, track.projectionDiff);
		hTotSD3->Fill(track.totSD);
		hTotSDProjectionDiff[maxHitsLayer]->Fill(track.totSD, track.projectionDiff);
	}
	for (Track track : twoLayerTracks) {
		hTimeSD2->Fill(track.timeSD);
		hTotSD2->Fill(track.totSD);
	}
	// plot to hTimeDiff
	for (Track track : threeLayerTracks) {
		for (Int_t layer = 0; layer < 3; layer++) {
			hTimeDiff[layer]->Fill(track.time[(layer + 1) % 3] - track.time[(layer + 2) % 3]);
		}
	}
	// plot to hTrackDistance
	if (threeLayerTracks.size() == 2) {
		for (Int_t layer = 0; layer < 3; layer++) {
			Double_t distance = sqrt(
				pow(threeLayerTracks[0].coord[layer][0] - threeLayerTracks[1].coord[layer][0], 2) +
				pow(threeLayerTracks[0].coord[layer][1] - threeLayerTracks[1].coord[layer][1], 2)
			);
			hTrackDistance[layer]->Fill(distance);
		}
	}

	// draw and save hits and tracks
	if (!saveGraph) {
		return 0;
	}
	// draw for events with total number of tracks
	if (totalTracks != 2) {
		return 0;
	}

	vector<TString> stats;

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
		
		// create cluster stats
		stats = {
			Form("event %lld (%c)", inData.tree->GetReadEntry(), LAYER_NAMES[layer])
		};
		TString clusterStat = "clusters: = ";
		for (vector<UInt_t> cluster : clusters[layer]) {
			clusterStat += vectorToString(cluster);
			clusterStat += ", ";
		}
		clusterStat.Remove(clusterStat.Length() - 1);
		clusterStat.Remove(clusterStat.Length() - 1);
		stats.push_back(clusterStat);
		addStats(hTotXi[layer], stats);

		hTotXi[layer]->Draw("AP");
		hTotXi[layer]->SetTitle(Form("tot vs xi%c;xi%c;tot", LAYER_NAMES[layer], LAYER_NAMES[layer]));
		hTotXi[layer]->GetXaxis()->SetLimits(0.5, LAYER_CHANNELS[layer] + 0.5);
		hTotXi[layer]->GetYaxis()->SetRangeUser(0, 3);
	}

	// draw timings of all hits
	cHits->cd(4);
	gPad->SetGridx();
	delete gTiming;
	gTiming = new TGraph();
	gTiming->SetMarkerStyle(20);
	gTiming->SetMarkerSize(0.5);
	gTiming->Draw("AP");
	for (Int_t layer = 0; layer < 3; layer++) {
		for (vector<UInt_t> cluster : clusters[layer]) {
			Int_t points = gTiming->GetN();
			Long64_t timing = (*inData.timeV[layer])[cluster[0]] - (*inData.timeGate)[0];
			gTiming->SetPoint(points, timing, points);

			auto *label = new TText(timing, points, Form("%c%d ", LAYER_NAMES[layer], cluster[0]));
			label->SetTextSize(0.03);
			label->SetTextAlign(32);
			label->Draw();
		}
	}
	gTiming->SetTitle("hit vs timing;timing [ps];hit");
	gTiming->GetXaxis()->SetLimits(-5000, 15000);
	gTiming->GetYaxis()->SetRangeUser(-0.5, layerHits[0] + layerHits[1] + layerHits[2] - 0.5);
	cHits->Print(outName);
	cHits->Clear();

	// draw axis and target and togaxsi window
	cHits->Divide(1, 1);
	cHits->cd(1);
	gPad->DrawFrame(xPosRange[0], yPosRange[0], xPosRange[1], yPosRange[1]);
    gTarget->SetFillStyle(0); // no fill
    gTarget->SetLineColor(kGray);
    gTarget->SetLineStyle(3); // dotted
    gTarget->SetLineWidth(1);
	gTarget->Draw();
	gWindow->SetFillStyle(0);  // no fill
	gWindow->SetLineColor(kGray);
	gWindow->SetLineStyle(3);  // dotted
	gWindow->SetLineWidth(1);
	gWindow->Draw();

	// draw all hits fibers
	for (vector<UInt_t> cluster : clusters[0]) {
		Double_t posX = xiToPosGlobal((*inData.xiV[0])[cluster[0]], 0);
		TLine xFiber(posX, yPosRange[0], posX, yPosRange[1]);
		xFiber.SetLineColor(kGray);
		xFiber.SetLineWidth(1);
		xFiber.DrawClone("SAME");
	}
	for (vector<UInt_t> cluster : clusters[1]) {
		Double_t posY = xiToPosGlobal((*inData.xiV[1])[cluster[0]], 1);
		TLine yFiber(xPosRange[0], posY, xPosRange[1], posY);
		yFiber.SetLineColor(kGray);
		yFiber.SetLineWidth(1);
		yFiber.DrawClone("SAME");
	}
	for (vector<UInt_t> cluster : clusters[2]) {
		TF1 uFiber(Form("uFiber_%u", cluster[0]),
			"[0]+[1]*x",
			xPosRange[0], xPosRange[1]
		);		
		// analytic values
		uFiber.SetParameters(
			(((*inData.xiV[2])[cluster[0]] - 160 - 0.5) * (-0.75) + U_OFFSET_GLOBAL) * sqrt(2),
			-1
		);
		uFiber.SetLineColor(kGray);
		uFiber.SetLineWidth(1);
		uFiber.DrawClone("SAME");
	}

	// draw tracks
	delete gHits;
	gHits = new TGraph();
	stats = {
		Form("event %lld", inData.tree->GetReadEntry()),
		"hits:=time std dev [ps]:"
	};
	gHits->SetPoint(0, reactionCoord[0], reactionCoord[1]);
	for (UInt_t i = 0; i < threeLayerTracks.size(); i++) {
		Track track = threeLayerTracks[i];
		// draw fiber hit points
		for (Int_t layer = 0; layer < 3; layer++) {
			gHits->SetPoint(gHits->GetN(), track.coord[layer][0], track.coord[layer][1]);
		}
		// draw togaxsi window point
		gHits->SetPoint(gHits->GetN(), track.togaxsiWindowCoord[0], track.togaxsiWindowCoord[1]);

		TLine trackLine3(reactionCoord[0], reactionCoord[1], track.coord[2][0], track.coord[2][1]);
		trackLine3.SetLineColor(kBlue);
		trackLine3.SetLineStyle(3); // dotted
		trackLine3.DrawClone("SAME");

		stats.push_back(
			Form("{%d, %d, %d} = %.0f", track.hit[0], track.hit[1], track.hit[2], track.timeSD)
		);
	}
	for (UInt_t i = 0; i < twoLayerTracks.size(); i++) {
		Track track = twoLayerTracks[i];
		TLine trackLine2;
		// draw fiber hit points
		Double_t x1 = track.coord[(noHitsLayer + 1) % 3][0];
		Double_t y1 = track.coord[(noHitsLayer + 1) % 3][1];
		Double_t x2 = track.coord[(noHitsLayer + 2) % 3][0];
		Double_t y2 = track.coord[(noHitsLayer + 2) % 3][1];
		gHits->SetPoint(gHits->GetN(), x1, y1);
		gHits->SetPoint(gHits->GetN(), x2, y2);
		// draw togaxsi window point
		gHits->SetPoint(gHits->GetN(), track.togaxsiWindowCoord[0], track.togaxsiWindowCoord[1]);

		trackLine2.SetX1(reactionCoord[0]);
		trackLine2.SetY1(reactionCoord[1]);
		if (noHitsLayer == 2) {
			trackLine2.SetX2(track.coord[1][0]);
			trackLine2.SetY2(track.coord[1][1]);
		} else {
			trackLine2.SetX2(track.coord[2][0]);
			trackLine2.SetY2(track.coord[2][1]);
		}

		trackLine2.SetLineColor(kRed);
		trackLine2.SetLineStyle(3); // dotted
		trackLine2.DrawClone("SAME");

		if (track.projectedLayer == 2) {
			stats.push_back(
				Form(
					"{%d, %d, X} = %.0f", 
					track.hit[0], 
					track.hit[1], 
					track.timeSD
				)
			);
		} else if (track.projectedLayer == 0) {
			stats.push_back(
				Form(
					"{X, %d, %d} = %.0f", 
					track.hit[1], 
					track.hit[2],
					track.timeSD
				)
			);
		} else {
			stats.push_back(
				Form(
					"{%d, X, %d} = %.0f", 
					track.hit[0], 
					track.hit[2],
					track.timeSD
				)
			);
		}
	}
	addStats(gHits, stats);
	gHits->SetMarkerStyle(20);
	gHits->SetMarkerSize(0.4);
	gHits->SetTitle("; x [mm]; y[mm]");
	gHits->DrawClone("P SAME"); // redraw points on top

	// save graph
	cHits->Print(outName);
	cHits->Clear();

	return 1;
}

void printTracksEnd(
	const DataBase& inData,
	Int_t clusterSize,
	Bool_t windowed,
	Bool_t propagated,
	const TString& outName
) {
	// print hTrack
	Double_t totalEntries = hTracks->GetEntries();
	vector<TString> stats = {
		Form("%s", inData.runNum.Data()),
		Form("entries = %.0f", totalEntries)
	};
	array<Double_t, 5> tracksPercentage;
	array<Double_t, 4> threeLayerTracksEventsPercentage;
	tracksPercentage[0] = hTracks->GetBinContent(1) / totalEntries * 100;
	stats.push_back(Form("0 tracks = %.0f (%.2f%%)", hTracks->GetBinContent(1), tracksPercentage[0]));
	for (Int_t totalTracks = 1; totalTracks <= 4; totalTracks++) {
		tracksPercentage[totalTracks] = hTracks->GetBinContent(totalTracks + 1) / totalEntries * 100;
		threeLayerTracksEventsPercentage[totalTracks - 1] = threeLayerTracksEvents[totalTracks - 1] / totalEntries * 100;
		stats.push_back(
			Form("%d tracks = %.0f (%.2f%%)", totalTracks, hTracks->GetBinContent(totalTracks + 1), tracksPercentage[totalTracks])
		);
		stats.push_back(
			Form("  3LT only = %lld (%.2f%%)", threeLayerTracksEvents[totalTracks - 1], threeLayerTracksEventsPercentage[totalTracks - 1])
		);
	}
	addStats(hTracks, stats);
	cHits->SetGrid();
	hTracks->Draw();
	cHits->Print(outName);

	// print hProp
	for (Int_t layer = 0; layer < 4; layer++) {
		TGraph *gPeaks = new TGraph();

		// fit for each slice and print
		for (Int_t xbin = 1; xbin <= 320; xbin++) {
			TH1D *py = hProp[layer]->ProjectionY(Form("py_x%d", xbin), xbin, xbin);

			if (py->GetEntries() <= 500) {
				// cout << "Not enough entries for bin " << xbin << "!" << endl;
				continue;
			} 

			// fit
			Double_t peakMax = py->GetBinCenter(py->GetMaximumBin());
			TF1 gaus(Form("g_x%d", xbin), "gaus", peakMax - 1000, peakMax + 500);
			gaus.SetParameters(py->GetMaximum(), peakMax, py->GetRMS());
			py->Fit(&gaus, "QSR");

			// plot to gPeaks
			Double_t peak = gaus.GetParameter(1);
			gPeaks->SetPoint(gPeaks->GetN(), hProp[layer]->GetXaxis()->GetBinCenter(xbin), peak);
		}

		// fit the peaks
		gPeaks->Fit("pol1", "Q", "", -50, 50);
		TF1 *fitAll = gPeaks->GetFunction("pol1");
		fitAll->SetLineWidth(1);
		Double_t intercept = fitAll->GetParameter(0);
		Double_t slope = fitAll->GetParameter(1);

		addStats(hProp[layer], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hProp[layer]->GetEntries()), 
			Form("cluster size = %d", clusterSize), 
			Form("windowed = %s", windowed ? "yes" : "no"),
			Form("propagated = %s", propagated ? "yes" : "no"),
			Form("intercept = %.3f", intercept),
			Form("slope = %.3f", slope)
		});

		hProp[layer]->Draw();
		fitAll->Draw("SAME");
		gPeaks->Draw("PX SAME");

		cHits->SetLeftMargin(0.15);
		cHits->SetLogz();
		cHits->Print(outName);
		cHits->SetLogz(0);
	}

	// print hProjectionDiff
	for (Int_t layer = 0; layer < 3; layer++) {
		Double_t totalEntries = hProjectionDiff[layer]->GetEntries();
		if (totalEntries != 0) {
			hProjectionDiff[layer]->Fit("gaus");
			TF1 *fit = hProjectionDiff[layer]->GetFunction("gaus");
			addStats(hProjectionDiff[layer], {
				Form("%s", inData.runNum.Data()),
				Form("entries = %.0f", totalEntries), 
				Form("cluster size = %d", clusterSize), 
				Form("windowed = %s", windowed ? "yes" : "no"),
				Form("propagated = %s", propagated ? "yes" : "no"),
				Form("amp = %.4g", fit->GetParameter(0)),
				Form("mean = %.4g", fit->GetParameter(1)),
				Form("sigma = %.4g", fit->GetParameter(2))
			});
		}
		hProjectionDiff[layer]->Draw();
		cHits->Print(outName);
	}

	// print hTimeDiff
	for (Int_t layer = 0; layer < 3; layer++) {
		hTimeDiff[layer]->Fit("gaus");
		TF1 *fit = hTimeDiff[layer]->GetFunction("gaus");
		addStats(hTimeDiff[layer], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTimeDiff[layer]->GetEntries()),
			Form("cluster size = %d", clusterSize),
			Form("windowed = %s", windowed ? "yes" : "no"),
			Form("propagated = %s", propagated ? "yes" : "no"),
			Form("amp = %.4g", fit->GetParameter(0)),
			Form("mean = %.4g", fit->GetParameter(1)),
			Form("sigma = %.4g", fit->GetParameter(2))
		});
		hTimeDiff[layer]->Draw();
		cHits->Print(outName);
	}

	// print hTimeSD
	addStats(hTimeSD3, {
		Form("%s", inData.runNum.Data()),
		Form("entries = %.0f", hTimeSD3->GetEntries()),
		Form("cluster size = %d", clusterSize),
		Form("windowed = %s", windowed ? "yes" : "no"),
		Form("propagated = %s", propagated ? "yes" : "no")
	});
	hTimeSD3->Draw();
	cHits->Print(outName);
	addStats(hTimeSD2, {
		Form("%s", inData.runNum.Data()),
		Form("entries = %.0f", hTimeSD2->GetEntries()),
		Form("cluster size = %d", clusterSize),
		Form("windowed = %s", windowed ? "yes" : "no"),
		Form("propagated = %s", propagated ? "yes" : "no")
	});
	hTimeSD2->Draw();
	cHits->Print(outName);

	// print hTimeSDProjectionDiff and gTimeSDProjectionDiffSigma
	for (Int_t layer = 0; layer < 3; layer++) {
		addStats(hTimeSDProjectionDiff[layer], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTimeSDProjectionDiff[layer]->GetEntries()),
			Form("cluster size = %d", clusterSize),
			Form("windowed = %s", windowed ? "yes" : "no"),
			Form("propagated = %s", propagated ? "yes" : "no"),
		});
		hTimeSDProjectionDiff[layer]->Draw();
		cHits->SetLogz();
		cHits->Print(outName);
		cHits->SetLogz(0);
		for (Int_t xbin = 1; xbin <= 200; xbin++) {
			TH1D *py = hTimeSDProjectionDiff[layer]->ProjectionY(Form("py_x%d", xbin), xbin, xbin);

			if (py->GetEntries() <= 500) {
				// cout << "Not enough entries for bin " << xbin << "!" << endl;
				continue;
			} 

			// fit
			py->Fit("gaus");
			TF1 *fit = py->GetFunction("gaus");

			// plot to gPeaks
			gTimeSDProjectionDiffSigma[layer]->SetPoint(
				gTimeSDProjectionDiffSigma[layer]->GetN(), 
				hTimeSDProjectionDiff[layer]->GetXaxis()->GetBinCenter(xbin),
				fit->GetParameter(2)
			);
		}

		gTimeSDProjectionDiffSigma[layer]->SetMarkerStyle(20);
		gTimeSDProjectionDiffSigma[layer]->SetMarkerSize(0.8);
		gTimeSDProjectionDiffSigma[layer]->SetLineWidth(2);
		gTimeSDProjectionDiffSigma[layer]->Draw("AP");
		cHits->Print(outName);
	}

	// print hTotSD
	addStats(hTotSD3, {
		Form("%s", inData.runNum.Data()),
		Form("entries = %.0f", hTotSD3->GetEntries()),
		Form("cluster size = %d", clusterSize),
		Form("windowed = %s", windowed ? "yes" : "no"),
		Form("propagated = %s", propagated ? "yes" : "no")
	});
	hTotSD3->Draw();
	cHits->Print(outName);
	addStats(hTotSD2, {
		Form("%s", inData.runNum.Data()),
		Form("entries = %.0f", hTotSD2->GetEntries()),
		Form("cluster size = %d", clusterSize),
		Form("windowed = %s", windowed ? "yes" : "no"),
		Form("propagated = %s", propagated ? "yes" : "no")
	});
	hTotSD2->Draw();
	cHits->Print(outName);

	// print hTotSDProjectionDiff and gTotSDProjectionDiffSigma
	for (Int_t layer = 0; layer < 3; layer++) {
		addStats(hTotSDProjectionDiff[layer], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTotSDProjectionDiff[layer]->GetEntries()),
			Form("cluster size = %d", clusterSize),
			Form("windowed = %s", windowed ? "yes" : "no"),
			Form("propagated = %s", propagated ? "yes" : "no"),
		});
		hTotSDProjectionDiff[layer]->Draw();
		cHits->SetLogz();
		cHits->Print(outName);
		cHits->SetLogz(0);
		for (Int_t xbin = 1; xbin <= 200; xbin++) {
			TH1D *py = hTotSDProjectionDiff[layer]->ProjectionY(Form("py_x%d", xbin), xbin, xbin);

			if (py->GetEntries() <= 500) {
				// cout << "Not enough entries for bin " << xbin << "!" << endl;
				continue;
			} 

			// fit
			py->Fit("gaus");
			TF1 *fit = py->GetFunction("gaus");

			// plot to gPeaks
			gTotSDProjectionDiffSigma[layer]->SetPoint(
				gTotSDProjectionDiffSigma[layer]->GetN(), 
				hTotSDProjectionDiff[layer]->GetXaxis()->GetBinCenter(xbin),
				fit->GetParameter(2)
			);
		}

		gTotSDProjectionDiffSigma[layer]->SetMarkerStyle(20);
		gTotSDProjectionDiffSigma[layer]->SetMarkerSize(0.8);
		gTotSDProjectionDiffSigma[layer]->SetLineWidth(2);
		gTotSDProjectionDiffSigma[layer]->Draw("AP");
		cHits->Print(outName);
	}

	// print hTrackDistance
	for (Int_t layer = 0; layer < 3; layer++) {
		addStats(hTrackDistance[layer], {
			Form("%s", inData.runNum.Data()),
			Form("entries = %.0f", hTrackDistance[layer]->GetEntries()),
			Form("cluster size = %d", clusterSize),
			Form("windowed = %s", windowed ? "yes" : "no"),
			Form("propagated = %s", propagated ? "yes" : "no")
		});
		hTrackDistance[layer]->Draw();
		cHits->Print(outName);
	}

	cHits->Print(Form("%s]", outName.Data()));
}

void printTracks(
	const vector<TString>& inPaths,
	Int_t clusterSize = 0,
	Bool_t windowed = 1,
	Bool_t propagated = 1,
	Long64_t printEvents = 100
) {
	// load data
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	TString outName = Form("%s_tracks_clusterSize%d", inData.runNum.Data(), clusterSize);
	outName += windowed ? "_windowed" : "_unwindowed";
	outName += propagated ? "_propagated" : "_unpropagated";
	outName += ".pdf";
	printTracksStart(inData, outName);

	// event loop
	Long64_t printedEvents = 0;
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		if (printedEvents < printEvents) {
			printedEvents += printTracksLoop(inData, TARGET_CENTER, clusterSize, windowed, propagated, outName, 1);
		} else {
			printTracksLoop(inData, TARGET_CENTER, clusterSize, windowed, propagated, outName, 0);
		}
	}

	printTracksEnd(inData, clusterSize, windowed, propagated, outName);
}

