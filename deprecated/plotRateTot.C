#include <TChain.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>
#include <TSystem.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TROOT.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <vector>
#include <string>

#include "utils/parameters.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

// const char* beamProfileRunNum 		= "0029";
// const char* graphName		 		= "200kHz";
const char* beamProfileRunNum 		= "0035";
const char* graphName		 		= "1MHz";

const Float_t rateRange[2] 			= {0, 10e6};
const Float_t rateBins 				= 100;
const Float_t totRange[2] 			= {50e3, 400e3};
const Float_t totBins 				= 200;
const Float_t totFitRange[2] 		= {50e3, 180e3};

TH2F* hRateTot = nullptr;
TH2F* hRateTotNorm = nullptr;
TGraphErrors* gMean = nullptr;
vector <vector <TH2F*>> hRateTotChannel(3);
vector <vector <TH2F*>> hRateTotChannelNorm(3);
vector <vector <TGraphErrors*>> gMeanChannels(3);

void plotRateTot(TChain* inputChain) {
	Float_t 					rate;
	vector<vector <Float_t>*> 	totVectors(3, nullptr);
	vector<vector <Int_t>*> 	xiVectors(3, nullptr);
	inputChain->SetBranchStatus("*", 0);
	inputChain->SetBranchStatus("rate", 1);
	inputChain->SetBranchAddress("rate", &rate);
	for (Int_t i = 0; i < 3; i++) {
		inputChain->SetBranchStatus(Form("tot%s", LAYERS[i]), 1);
		inputChain->SetBranchAddress(Form("tot%s", LAYERS[i]), &totVectors[i]);
	}
	
	// initialize graphs
	cout << "Intializing graphs..." << endl;
	hRateTot = new TH2F(
		"hRateTot", "hRateTot", 
		rateBins, rateRange[0], rateRange[1], 
		totBins, totRange[0], totRange[1]
	);
	gMean = new TGraphErrors();

	// fill graphs
	Long64_t events = inputChain->GetEntries();
	for (Long64_t event = 0; event < events; event++) {
		printProgress(event, events);

		inputChain->GetEntry(event);
		for (Int_t layer = 0; layer < 3; layer++) {
			for (UInt_t entry = 0; entry < totVectors[layer]->size(); entry++) {
				if (rate == 0) continue;
				hRateTot->Fill(rate, (*totVectors[layer])[entry]);
			}
		}
	}
	cout << endl;

	// normalize and fit
	cout << "Normalizing and fitting..." << endl;
	hRateTotNorm = (TH2F*) hRateTot->Clone("hRateTotNorm");
	TF1* fGaus = new TF1("fGaus", "gaus", totFitRange[0], totFitRange[1]);
	int point = 0;
	for (Int_t rateBin = 1; rateBin <= rateBins; rateBin++) {
		// normalize
		Float_t stripIntegral = 0.0;
		for (Int_t totBin = 1; totBin <= totBins; totBin++) {
			stripIntegral += hRateTot->GetBinContent(rateBin, totBin);
		}
		if (stripIntegral == 0) continue;
		for (Int_t totBin = 1; totBin <= totBins; totBin++) {
			Float_t value = hRateTot->GetBinContent(rateBin, totBin);
			hRateTotNorm->SetBinContent(rateBin, totBin, value / stripIntegral);
		}

		// fit
		TH1D* py = hRateTotNorm->ProjectionY(Form("py_%d", rateBin), rateBin, rateBin);
		fGaus->SetParameters(py->GetMaximum(), (totFitRange[0] + totFitRange[1])/2, py->GetRMS());
		py->Fit(fGaus, "Q0R");	// Fit quietly, don’t draw fit, don’t spam output
		double mean    = fGaus->GetParameter(1);
		double meanErr = fGaus->GetParError(1);
		double xCenter = hRateTotNorm->GetXaxis()->GetBinCenter(rateBin);
		gMean->SetPoint(point, xCenter, mean);
		// gMean->SetPointError(point, 0.0, meanErr);
		point++;
		delete py;
	}
	// Draw
	hRateTotNorm->Draw("COLZ");
	gMean->SetMarkerStyle(20);	// circle marker
	gMean->SetMarkerSize(0.5);
	gMean->SetMarkerColor(kRed);
	gMean->Draw("P SAME");
}

void plotRateTotChannel(TChain* inputChain) {
	gROOT->SetBatch(kTRUE);

	Float_t 					rate;
	vector<vector <Float_t>*> 	totVectors(3, nullptr);
	vector<vector <Int_t>*> 	xiVectors(3, nullptr);
	inputChain->SetBranchStatus("*", 0);
	inputChain->SetBranchStatus("rate", 1);
	inputChain->SetBranchAddress("rate", &rate);
	for (Int_t i = 0; i < 3; i++) {
		inputChain->SetBranchStatus(Form("tot%s", LAYERS[i]), 1);
		inputChain->SetBranchAddress(Form("tot%s", LAYERS[i]), &totVectors[i]);
		inputChain->SetBranchStatus(Form("xi%s", LAYERS[i]), 1);
		inputChain->SetBranchAddress(Form("xi%s", LAYERS[i]), &xiVectors[i]);
	}

	// load beam profile
	vector <vector <Float_t>> beamProfile(3);
	for (Int_t layer = 0; layer < 3; layer++) {
		ifstream tsv(Form("beamProfile%s_%s.tsv", LAYERS[layer], beamProfileRunNum));
		string line;

		// skip header
		getline(tsv, line);
		beamProfile[layer].push_back(0);

		while (getline(tsv, line)) {
			istringstream ss(line);
			Int_t xiBin;
			Float_t contribution;
			ss >> xiBin >> contribution;

			beamProfile[layer].push_back(contribution);
		}
	}
	
	// initialize graphs
	cout << "Intializing graphs..." << endl;
	for (Int_t layer = 0; layer < 3; layer++) {
		hRateTotChannel[layer].resize(LAYER_CHANNELS[layer]);
		hRateTotChannelNorm[layer].resize(LAYER_CHANNELS[layer]);
		gMeanChannels[layer].resize(LAYER_CHANNELS[layer]);
		for (Int_t channel = 0; channel < LAYER_CHANNELS[layer]; channel++) {
			hRateTotChannel[layer][channel] = new TH2F(
				Form("hRateTotChannel%s_%d", LAYERS[layer], channel), 
				Form("hRateTotChannel%s_%d", LAYERS[layer], channel), 
				rateBins, rateRange[0] / 4e2, rateRange[1] / 4e2, 
				totBins, totRange[0], totRange[1]
			);
			hRateTotChannelNorm[layer][channel] = new TH2F(
				Form("hRateTotChannelNorm%s_%d", LAYERS[layer], channel), 
				Form("hRateTotChannelNorm%s_%d", LAYERS[layer], channel), 
				rateBins, rateRange[0] / 4e2, rateRange[1] / 4e2, 
				totBins, totRange[0], totRange[1]
			);
		}
	}

	// fill graphs
	Long64_t events = inputChain->GetEntries();
	for (Long64_t event = 0; event < events; event++) {
		printProgress(event, events);

		inputChain->GetEntry(event);
		for (Int_t layer = 0; layer < 3; layer++) {
			for (UInt_t entry = 0; entry < totVectors[layer]->size(); entry++) {
				if (rate == 0) continue;
				hRateTotChannel[layer][(*xiVectors[layer])[entry] - 1]->Fill(
					rate * beamProfile[layer][(*xiVectors[layer])[entry]], (*totVectors[layer])[entry]
				);
			}
		}
	}
	cout << endl;

	// normalize and fit
	cout << "Normalizing and fitting for channels..." << endl;
	TF1* fGaus = new TF1("fGaus", "gaus", totFitRange[0], totFitRange[1]);
	int point = 0;
	for (Int_t layer = 0; layer < 3; layer++) {
		const TString graph_fname = Form("hRateTotChannel%s_%s.pdf", LAYERS[layer], graphName);
		// const TString graph_fname = Form("hRateTot%s_1MHz.pdf", LAYERS[layer]);
		TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
		c1->Print(Form("%s[", graph_fname.Data()));
		for (Int_t channel = 0; channel < LAYER_CHANNELS[layer]; channel++) {
			hRateTotChannelNorm[layer][channel] = (TH2F*) hRateTotChannel[layer][channel]->Clone(
				Form("hRateTotChannelNorm%s_%d", LAYERS[layer], channel)
			);
			gMeanChannels[layer][channel] = new TGraphErrors();
			point = 0;
			for (Int_t rateBin = 1; rateBin <= rateBins; rateBin++) {
				// normalize
				Float_t stripIntegral = 0.0;
				for (Int_t totBin = 1; totBin <= totBins; totBin++) {
					stripIntegral += hRateTotChannel[layer][channel]->GetBinContent(rateBin, totBin);
				}
				if (stripIntegral == 0) continue;
				for (Int_t totBin = 1; totBin <= totBins; totBin++) {
					Float_t value = hRateTotChannel[layer][channel]->GetBinContent(rateBin, totBin);
					hRateTotChannelNorm[layer][channel]->SetBinContent(rateBin, totBin, value / stripIntegral);
				}
				// fit
				TH1D* py = hRateTotChannelNorm[layer][channel]->ProjectionY(Form("py_%d", rateBin), rateBin, rateBin);
				fGaus->SetParameters(py->GetMaximum(), (totFitRange[0] + totFitRange[1])/2, py->GetRMS());
				py->Fit(fGaus, "Q0R");	// Fit quietly, don’t draw fit, don’t spam output
				double mean    = fGaus->GetParameter(1);
				double meanErr = fGaus->GetParError(1);
				double xCenter = hRateTotChannelNorm[layer][channel]->GetXaxis()->GetBinCenter(rateBin);
				gMeanChannels[layer][channel]->SetPoint(point, xCenter, mean);
				gMeanChannels[layer][channel]->SetPointError(point, 0.0, meanErr);
				point++;
				delete py;
			}
			// draw and save
			hRateTotChannelNorm[layer][channel]->Draw("COLZ");
			gMeanChannels[layer][channel]->SetMarkerStyle(20);
			gMeanChannels[layer][channel]->SetMarkerSize(0.3);
			gMeanChannels[layer][channel]->SetMarkerColor(kRed);
			gMeanChannels[layer][channel]->Draw("P SAME");
			c1->SetGrid();
			c1->Print(graph_fname);
		}
		c1->Print(Form("%s]", graph_fname.Data()));
		delete c1;
	}
}
