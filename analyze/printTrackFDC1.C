#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TString.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TH1F.h>
#include <TF1.h>
#include <TLine.h>
#include <TAxis.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <map>
#include <vector>

#include "utils/loadData.C"
#include "utils/printProgress.C"

TH1F* hdPosX = nullptr;

void printTrackFDC1(const TString& inPath1, const TString& inPath2) {
	// constants
	const char layersFBT[3] = {'X', 'Y', 'U'};
	const Int_t zPosFBT = -700;
	const vector<Float_t> xRangeFBT = {(0 - 2.5 - 160) * 0.75, (320 - 2.5 - 160) * 0.75};
	const vector<Int_t> layersFDC1 = {65, 55, 5, -5, -55, -65}; // X layers
	const vector<Int_t> zRangeFDC1 = {-65, 65};
	const bool selective = 0;

	// graph settings
	hdPosX = new TH1F("hdPosX", "hdPosX", 50, -1 * (xRangeFBT[1] - xRangeFBT[0]), xRangeFBT[1] - xRangeFBT[0]);
	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	TString runNumber = TString(gSystem->BaseName(inPath1))(0,4);
	TString graphPath;
	TString graphPath2;
	if (selective) {
		graphPath = Form("%s_track_selective.pdf", runNumber.Data());
		graphPath2 = Form("%s_hdPosX_selective.pdf", runNumber.Data());
	} else {
		graphPath = Form("%s_track.pdf", runNumber.Data());
		graphPath2 = Form("%s_hdPosX.pdf", runNumber.Data());
	}
	c1->SetGrid();
	// c1->Print(Form("%s[", graphPath.Data()));

	// draw FBT range overlay
	TLine *lFBT = new TLine(zPosFBT, xRangeFBT[0], zPosFBT, xRangeFBT[1]);
	lFBT->SetLineColor(kBlue);
	lFBT->SetLineWidth(2);

	DataFBT2 inData1({inPath1}, "events");
	inData1.tree->SetBranchStatus("*", 0);
	inData1.tree->SetBranchStatus("xiX", 1);
	inData1.tree->SetBranchStatus("xiY", 1);
	inData1.tree->SetBranchStatus("xiU", 1);
	inData1.tree->SetBranchStatus("totX", 1);
	inData1.tree->SetBranchStatus("totY", 1);
	inData1.tree->SetBranchStatus("totU", 1);

	DataFDC1 inData2({inPath2}, "tree");
	inData2.tree->SetBranchStatus("*", 0);
	inData2.tree->SetBranchStatus("ts", 1);
	inData2.tree->SetBranchStatus("fdc1ht.wirez", 1);
	inData2.tree->SetBranchStatus("fdc1ht.wirepos", 1);

	for (Long64_t entry = 0; entry < inData1.entries; entry++) {
	// for (Long64_t entry = 0; entry < 5000; entry++) {
		printProgress(entry, inData1.entries);

		inData1.tree->GetEntry(entry);
		inData2.tree->GetEntry(entry);

		// continue if no FBT xiX hits
		if (inData1.xiV[0]->size() == 0) {
			continue;
		}
		// selective
		if (selective && !(inData1.xiV[0]->size() > 0 && inData1.xiV[1]->size() > 0 && inData1.xiV[2]->size() > 0)) {
			continue;
		}
		if (selective && !((*inData1.totV[0])[0] > 20e3 && (*inData1.totV[1])[0] > 20e3 && (*inData1.totV[2])[0] > 20e3)) {
			continue;
		}

		// continue is no FDC1 X layers hits
		Int_t mult = inData2.tree->GetLeaf("fdc1ht.wirez")->GetLen();
		if (mult == 0) {
			continue;
		}
		map<Int_t, Float_t> layersHitFDC1;
		for (Int_t i = zRangeFDC1[0]; i <= zRangeFDC1[1]; i += 5) {
			layersHitFDC1[i] = 9999;
		}
		for (Int_t i = 0; i < mult; i++) {
			layersHitFDC1[inData2.wirez[i]] = inData2.wirepos[i];
		}
		Int_t hitCountFDC1 = 0;
		for (UInt_t i = 0; i < layersFDC1.size(); i++) {
			if (layersHitFDC1[layersFDC1[i]] != 9999) {
				hitCountFDC1++;
			}
		}
		if (hitCountFDC1 == 0) {
			continue;
		}

		// get FBT alignment
		TString layersHitFBT = "";
		Float_t alignment = 0;
		for (Int_t i = 0; i < 3; i++) {
			if (inData1.xiV[i]->size() > 0) {
				layersHitFBT += layersFBT[i];
			}
		}
		if (layersHitFBT == "XYU") {
			alignment = (*inData1.xiV[0])[0] + (*inData1.xiV[1])[0] - ((*inData1.xiV[2])[0] + 70)/0.83;
		}
		Float_t xPosFBT = ((*inData1.xiV[0])[0] - 2.5 - 160) * 0.75;

		// draw tracking graph
		TGraph *gHits = new TGraph();
		UInt_t nPoint = 0;
		gHits->SetPoint(nPoint, zPosFBT, xPosFBT);
		nPoint++;
		for (UInt_t i = 0; i < layersFDC1.size(); i++) {
			if (layersHitFDC1[layersFDC1[i]] == 9999) {
				continue;
			}
			gHits->SetPoint(nPoint, layersFDC1[i], layersHitFDC1[layersFDC1[i]]);
			nPoint++;
		}
		gHits->SetMarkerStyle(20);	// circle marker
		gHits->SetMarkerSize(1.0);
		gHits->SetMarkerColor(kRed);
		gHits->Draw("AP");
		gHits->GetXaxis()->SetLimits(-800, 100);
		gHits->SetMinimum(-300);
		gHits->SetMaximum(300);
		gHits->SetTitle(Form(
			"Entry=%lld, FBT: %s (alignment=%.1f, TOT=%.1f[ns], xPosDiff=);;[mm]", 
			entry, layersHitFBT.Data(), alignment, (*inData1.totV[0])[0]/1e3
		));
		lFBT->Draw("same");

		// fit FDC1 track if 3 or more hits
		if (hitCountFDC1 >= 3) {
			TF1 *fFit = new TF1("fFit", "pol1", zRangeFDC1[0], zRangeFDC1[1]);
			gHits->Fit(fFit, "RQ0");
			fFit->SetRange(-800, 100);
			fFit->SetLineColor(kBlack);
			fFit->SetLineWidth(1);
			fFit->SetLineStyle(2);
			fFit->Draw("same");

			Float_t xPosFBTFit = fFit->GetParameter(1) * zPosFBT + fFit->GetParameter(0);
			if (xPosFBTFit >= xRangeFBT[0] && xPosFBTFit <= xRangeFBT[1]) {
				gHits->SetTitle(Form(
					"Entry=%lld, FBT: %s (alignment=%.1f, TOT=%.1f[ns], xPosDiff=%.1f[mm]);;[mm]", 
					entry, layersHitFBT.Data(), alignment, (*inData1.totV[0])[0]/1e3, xPosFBT - xPosFBTFit
				));
				if ((*inData1.totV[0])[0] > 20e3) {
					hdPosX->Fill(xPosFBT - xPosFBTFit);
				}
			}
		}

		// print and clear graph
		// cout << "printing for entry " << entry << endl;
		// c1->Print(graphPath);
		gHits->Set(0);
		delete gHits;
		gHits = nullptr;
	}
	// c1->Print(Form("%s]", graphPath.Data()));
	c1->Clear();
	hdPosX->Draw();
	c1->Print(graphPath2);
}

