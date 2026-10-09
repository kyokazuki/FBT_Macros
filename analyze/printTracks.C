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
#include <TLine.h>
#include <TF1.h>
#include <TText.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/track.C"

vector<TH1F*> hProjectionDiff(3, nullptr);
vector<TH1F*> hTrackDistance(3, nullptr);
TH1F* hTimeSD3;
TH1F* hTimeSD2;

TCanvas *cHits = nullptr;
vector<TGraph*> hTotXi(3);
TGraph *gTiming;
TEllipse *gTarget = nullptr;
TGraph *gHits;

Double_t xPosRange[2] = {xiToPosGlobal(320, 0), xiToPosGlobal(1, 0)};
Double_t yPosRange[2] = {xiToPosGlobal(224, 1), xiToPosGlobal(1, 1)};

void printTracksStart(
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

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hProjectionDiff[layer];
		hProjectionDiff[layer] = new TH1F(
			Form("hProjectionDiff%c", LAYER_NAMES[layer]),
			Form("hProjectionDiff%c; residual [mm]", LAYER_NAMES[layer]), 
			200, -3, 3
		);

		delete hTrackDistance[layer];
		hTrackDistance[layer] = new TH1F(
			Form("hTrackDistance%c", LAYER_NAMES[layer]),
			Form("hTrackDistance%c; residual [mm]", LAYER_NAMES[layer]), 
			200, 0, 200
		);
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

	// initiate objects
	delete cHits;
    cHits = new TCanvas("cHits", "cHits", 10*120, 7*120);
	for (Int_t i = 0; i < 3; i++) {
		delete hTotXi[i];
		hTotXi[i] = new TGraph();
	}
	delete gTarget;
    gTarget = new TEllipse(TARGET_CENTER[0], TARGET_CENTER[1], TARGET_RADIUS);

	cHits->Print(Form("%s[", outName.Data()));
}

Int_t printTracksLoop(
	const DataFBT2& inData, 
	const Double_t (&reactionCoord)[3], 
	Int_t clusterSize,
	const TString& outName,
	Bool_t saveGraph
) {
	// count hits in layers
	array<vector<UInt_t>, 3> hits;
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
			for (UInt_t i = 0; i < hits[layer].size(); i++) {
				Int_t xiDiff = abs((*inData.xiV[layer])[entry] - (*inData.xiV[layer])[hits[layer][i]]);
				if (xiDiff <= clusterSize) {
					isCluster = 1;
					clusters[layer][i].push_back(entry);
					break;
				}
			}
			if (!isCluster) {
				clusters[layer].push_back({entry});
				hits[layer].push_back(entry);
				layerHits[layer]++;
			}
		}

		if (hits[layer].size() == 0) {
			noHitsLayers++;
		}

		if (hits[layer].size() > maxHits) {
			maxHits = hits[layer].size();
			maxHitsLayer = layer;
		}
	}
	
	// set maxHitsLayer to U if possible
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
		for (UInt_t hit1 : hits[layer1]) {
			for (UInt_t hit2 : hits[layer2]) {
				Track track(
					maxHitsLayer,
					hit1,
					hit2,
					(*inData.xiV[layer1])[hit1],
					(*inData.xiV[layer2])[hit2],
					reactionCoord
				);
				// add the last layer
				for (UInt_t hit3 : hits[maxHitsLayer]) {
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
			if (!(
				abs(track.togaxsiWindowCoord[0]) < TOGAXSI_WINDOW_WIDTH / 2 && 
				abs(track.togaxsiWindowCoord[1]) < TOGAXSI_WINDOW_WIDTH / 2
			)) {
				continue;
			}

			// Check that these hits are still available
			bool hit1Available =
				find(
					hits[layer1].begin(),
					hits[layer1].end(),
					track.hit[layer1]
				) != hits[layer1].end();
			bool hit2Available =
				find(
					hits[layer2].begin(),
					hits[layer2].end(),
					track.hit[layer2]
				) != hits[layer2].end();
			bool hit3Available =
				find(
					hits[maxHitsLayer].begin(),
					hits[maxHitsLayer].end(),
					track.hit[maxHitsLayer]
				) != hits[maxHitsLayer].end();
			if (!hit1Available || !hit2Available || !hit3Available) {
				continue;
			}

			// Calculate timeSD then accept this track
			track.getTimeSD(
				(*inData.timeV[0])[track.hit[0]],
				(*inData.timeV[1])[track.hit[1]],
				(*inData.timeV[2])[track.hit[2]]
			);
			threeLayerTracks.push_back(track);
			hProjectionDiff[maxHitsLayer]->Fill(track.projectionDiff);
			hTimeSD3->Fill(track.timeSD);

			// Remove the three used hits
			hits[layer1].erase(
				remove(
					hits[layer1].begin(),
					hits[layer1].end(),
					track.hit[layer1]
				),
				hits[layer1].end()
			);
			hits[layer2].erase(
				remove(
					hits[layer2].begin(),
					hits[layer2].end(),
					track.hit[layer2]
				),
				hits[layer2].end()
			);
			hits[maxHitsLayer].erase(
				remove(
					hits[maxHitsLayer].begin(),
					hits[maxHitsLayer].end(),
					track.hit[maxHitsLayer]
				),
				hits[maxHitsLayer].end()
			);
		}

		// Recalculate number of empty layers
		noHitsLayers = 0;
		for (Int_t layer = 0; layer < 3; layer++) {
			if (hits[layer].empty()) {
				noHitsLayers++;
			}
		}
	}

	// extract 2-fiber tracks if only two fiber remains
	vector<Track> twoLayerTracks;
	Int_t noHitsLayer = -1;
	if (noHitsLayers == 1 && hits[0].size() + hits[1].size() + hits[2].size() == 2) {
		if (hits[0].size() == 0) {
			noHitsLayer = 0;
		} else if (hits[1].size() == 0) {
			noHitsLayer = 1;
		} else if (hits[2].size() == 0) {
			noHitsLayer = 2;
		}
		Int_t layer1 = (noHitsLayer + 1) % 3;
		Int_t layer2 = (noHitsLayer + 2) % 3;

		Track twoLayerTrack(
			noHitsLayer,
			hits[layer1][0],
			hits[layer2][0],
			(*inData.xiV[layer1])[hits[layer1][0]],
			(*inData.xiV[layer2])[hits[layer2][0]],
			reactionCoord
		);
		twoLayerTrack.getTimeSD(
			(*inData.timeV[layer1])[hits[layer1][0]],
			(*inData.timeV[layer2])[hits[layer2][0]]
		);
		// Check that track is within range
		if (inRange(twoLayerTrack.coord[0][0], xPosRange) && inRange(twoLayerTrack.coord[1][1], yPosRange)) {
			twoLayerTracks.push_back(twoLayerTrack);
			hTimeSD2->Fill(twoLayerTrack.timeSD);
		}
	}

	// plot if tracks are extracted
	if (threeLayerTracks.size() == 0 && twoLayerTracks.size() == 0) {
		return 0;
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
	vector<TString> stats;

	// temp
	if (threeLayerTracks.size() + twoLayerTracks.size() < 2) {
		return 0;
	}

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
		hTotXi[layer]->GetYaxis()->SetRangeUser(0, 5);
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
		// for (UInt_t hit = 0; hit < layerHits[layer]; hit++) {
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

	// draw frame and target
	cHits->Divide(1, 1);
	cHits->cd(1);
	gPad->DrawFrame(xPosRange[0], yPosRange[0], xPosRange[1], yPosRange[1]);
    gTarget->SetFillStyle(0);
    gTarget->SetLineStyle(1);
    gTarget->SetLineWidth(1);
    gTarget->SetLineColor(kGray);
	gTarget->Draw();

	// draw all hits fibers
	// for (UInt_t hit = 0; hit < layerHits[0]; hit++) {
	for (vector<UInt_t> cluster : clusters[0]) {
		Double_t posX = xiToPosGlobal((*inData.xiV[0])[cluster[0]], 0);
		TLine xFiber(posX, yPosRange[0], posX, yPosRange[1]);
		xFiber.SetLineColor(kGray);
		xFiber.SetLineWidth(1);
		xFiber.DrawClone("SAME");
	}
	// for (UInt_t hit = 0; hit < layerHits[1]; hit++) {
	for (vector<UInt_t> cluster : clusters[1]) {
		Double_t posY = xiToPosGlobal((*inData.xiV[1])[cluster[0]], 1);
		TLine yFiber(xPosRange[0], posY, xPosRange[1], posY);
		yFiber.SetLineColor(kGray);
		yFiber.SetLineWidth(1);
		yFiber.DrawClone("SAME");
	}
	// for (UInt_t hit = 0; hit < layerHits[2]; hit++) {
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
		"hits: = time std dev [ps]:"
	};
	gHits->SetPoint(0, reactionCoord[0], reactionCoord[1]);
	for (UInt_t i = 0; i < threeLayerTracks.size(); i++) {
		Track track = threeLayerTracks[i];
		for (Int_t layer = 0; layer < 3; layer++) {
			gHits->SetPoint(gHits->GetN(), track.coord[layer][0], track.coord[layer][1]);
		}

		TLine trackLine3(reactionCoord[0], reactionCoord[1], track.coord[2][0], track.coord[2][1]);
		trackLine3.SetLineColor(kBlue);
		trackLine3.SetLineStyle(3);
		trackLine3.DrawClone("SAME");

		stats.push_back(
			Form("{%d, %d, %d} = %.0f", track.hit[0], track.hit[1], track.hit[2], track.timeSD)
		);
	}
	for (UInt_t i = 0; i < twoLayerTracks.size(); i++) {
		Track track = twoLayerTracks[i];
		TLine trackLine2;
		Double_t x1 = track.coord[(noHitsLayer + 1) % 3][0];
		Double_t y1 = track.coord[(noHitsLayer + 1) % 3][1];
		Double_t x2 = track.coord[(noHitsLayer + 2) % 3][0];
		Double_t y2 = track.coord[(noHitsLayer + 2) % 3][1];
		gHits->SetPoint(gHits->GetN(), x1, y1);
		gHits->SetPoint(gHits->GetN(), x2, y2);

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
		trackLine2.SetLineStyle(3);
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
	const TString& outName
) {
	TF1* fit;
	for (Int_t layer = 0; layer < 3; layer++) {
		Double_t totalEntries = hProjectionDiff[layer]->GetEntries();
		if (totalEntries != 0) {
			hProjectionDiff[layer]->Fit("gaus");
			fit = hProjectionDiff[layer]->GetFunction("gaus");
			addStats(hProjectionDiff[layer], {
				Form("run%s", inData.runNum.Data()),
				Form("entries = %.0f", totalEntries), 
				Form("cluster size = %d", clusterSize), 
				Form("amp = %.4g", fit->GetParameter(0)),
				Form("mean = %.4g", fit->GetParameter(1)),
				Form("sigma = %.4g", fit->GetParameter(2))
			});
		}

		cHits->SetGrid();
		hProjectionDiff[layer]->Draw();

		cHits->Print(outName);
	}

	addStats(hTimeSD3, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hTimeSD3->GetEntries()),
		Form("cluster size = %d", clusterSize)
	});
	cHits->SetGrid();
	hTimeSD3->Draw();
	cHits->Print(outName);

	addStats(hTimeSD2, {
		Form("run%s", inData.runNum.Data()),
		Form("entries = %.0f", hTimeSD2->GetEntries()),
		Form("cluster size = %d", clusterSize)
	});
	cHits->SetGrid();
	hTimeSD2->Draw();
	cHits->Print(outName);

	for (Int_t layer = 0; layer < 3; layer++) {
		addStats(hTrackDistance[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", hTrackDistance[layer]->GetEntries()),
			Form("cluster size = %d", clusterSize)
		});
		cHits->SetGrid();
		hTrackDistance[layer]->Draw();
		cHits->Print(outName);
	}

	cHits->Print(Form("%s]", outName.Data()));
}

void printTracks(
	const vector<TString>& inPaths,
	Int_t clusterSize = 0,
	Long64_t printEvents = 100
) {
	// load data
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	TString outName = Form("%s_tracks_clusterSize%d.pdf", inData.runNum.Data(), clusterSize);
	printTracksStart(inData, outName);

	// only draw printEvents of events
	Long64_t drawnEvents = 0;

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		if (drawnEvents < printEvents) {
			drawnEvents += printTracksLoop(inData, TARGET_CENTER, clusterSize, outName, 1);
		} else {
			printTracksLoop(inData, TARGET_CENTER, clusterSize, outName, 0);
		}
	}

	printTracksEnd(inData, clusterSize, outName);
}

