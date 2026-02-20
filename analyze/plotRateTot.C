#include <TChain.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TString.h>
#include <TSystem.h>
#include <TF1.h>
#include <TGraphErrors.h>

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>

TChain* chain = nullptr;
TH2F* hRateTot = nullptr;
TH2F* hRateTotNorm = nullptr;
TGraphErrors* gMean = nullptr;
vector <vector <TH2F*>> hRateTotChannel(3);
vector <vector <TH2F*>> hRateTotChannelNorm(3);
vector <vector <TGraphErrors*>> gMeanChannels(3);

// const vector <string> files = {
// 	"0034_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root"
// };
const vector <string> files = {
	"0028_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0029_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0030_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0031_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0032_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0033_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0034_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root"
};
// const vector <string> files = {
// 	"0035_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
// 	"0036_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
// 	"0037_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
// 	"0038_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
// 	"0040_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
// 	"0041_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
// 	"0042_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
// 	"0044_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
// 	"0046_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
// 	"0047_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
// 	"0048_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
// 	"0049_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
// 	"0050_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root"
// };
const Float_t rateRange[2] = {0, 10e6};
const Float_t rateBins = 200;
const Float_t totRange[2] = {50e3, 200e3};
const Float_t totBins = 100;
const Float_t totFitRange[2] = {50e3, 200e3};
const char* layers[3] = {"X","Y","U"};
const Int_t layerChannels[3] = {320, 224, 320};

void plotRateTot() {
	// initialize chain
	cout << "Loading trees..." << endl;
	chain = new TChain("events");
	for (const string& file : files) {
		chain->Add(file.c_str());
	}
	Float_t 					rate;
	vector<vector <Float_t>*> 	totVectors(3, nullptr);
	vector<vector <Int_t>*> 	xiVectors(3, nullptr);
	chain->SetBranchStatus("*", 0);
	chain->SetBranchStatus("rate", 1);
	chain->SetBranchAddress("rate", &rate);
	for (Int_t i = 0; i < 3; i++) {
		chain->SetBranchStatus(Form("tot%s", layers[i]), 1);
		chain->SetBranchAddress(Form("tot%s", layers[i]), &totVectors[i]);
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
	Long64_t events = chain->GetEntries();
	for (Long64_t event = 0; event < events; event++) {
		if (event % 10000 == 0 || event == events - 1) {
			cout << "\rEvent: " << event + 1 << "/" << events << flush;
		}
		chain->GetEntry(event);
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
		gMean->SetPointError(point, 0.0, meanErr);
		point++;
		delete py;
	}
	// Draw
	hRateTotNorm->Draw("COLZ");
	gMean->SetMarkerStyle(20);	// circle marker
	gMean->SetMarkerSize(0.3);
	gMean->SetMarkerColor(kRed);
	gMean->Draw("P SAME");
}

void plotRateTotChannel() {
	gROOT->SetBatch(kTRUE);
	// initialize chain
	cout << "Loading trees..." << endl;
	chain = new TChain("events");
	for (const string& file : files) {
		chain->Add(file.c_str());
	}
	Float_t 					rate;
	vector<vector <Float_t>*> 	totVectors(3, nullptr);
	vector<vector <Int_t>*> 	xiVectors(3, nullptr);
	chain->SetBranchStatus("*", 0);
	chain->SetBranchStatus("rate", 1);
	chain->SetBranchAddress("rate", &rate);
	for (Int_t i = 0; i < 3; i++) {
		chain->SetBranchStatus(Form("tot%s", layers[i]), 1);
		chain->SetBranchAddress(Form("tot%s", layers[i]), &totVectors[i]);
		chain->SetBranchStatus(Form("xi%s", layers[i]), 1);
		chain->SetBranchAddress(Form("xi%s", layers[i]), &xiVectors[i]);
	}
	
	// initialize graphs
	cout << "Intializing graphs..." << endl;
	for (Int_t layer = 0; layer < 3; layer++) {
		hRateTotChannel[layer].resize(layerChannels[layer]);
		hRateTotChannelNorm[layer].resize(layerChannels[layer]);
		gMeanChannels[layer].resize(layerChannels[layer]);
		for (Int_t channel = 0; channel < layerChannels[layer]; channel++) {
			hRateTotChannel[layer][channel] = new TH2F(
				Form("hRateTot%s_%d", layers[layer], channel), 
				Form("hRateTot%s_%d", layers[layer], channel), 
				rateBins, rateRange[0], rateRange[1], 
				totBins, totRange[0], totRange[1]
			);
			hRateTotChannelNorm[layer][channel] = new TH2F(
				Form("hRateTotNorm%s_%d", layers[layer], channel), 
				Form("hRateTotNorm%s_%d", layers[layer], channel), 
				rateBins, rateRange[0], rateRange[1], 
				totBins, totRange[0], totRange[1]
			);
		}
	}

	// fill graphs
	Long64_t events = chain->GetEntries();
	for (Long64_t event = 0; event < events; event++) {
		if (event % 10000 == 0 || event == events - 1) {
			cout << "\rEvent: " << event + 1 << "/" << events << flush;
		}
		chain->GetEntry(event);
		for (Int_t layer = 0; layer < 3; layer++) {
			for (UInt_t entry = 0; entry < totVectors[layer]->size(); entry++) {
				if (rate == 0) continue;
				hRateTotChannel[layer][(*xiVectors[layer])[entry] - 1]->Fill(
					rate, (*totVectors[layer])[entry]
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
		cout << "Layer: " << layer << endl;
		// const TString graph_fname = Form("hRateTot%s_200kHz.pdf", layers[layer]);
		const TString graph_fname = Form("hRateTot%s_1MHz.pdf", layers[layer]);
		TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
		c1->Print(Form("%s[", graph_fname.Data()));
		for (Int_t channel = 0; channel < layerChannels[layer]; channel++) {
			cout << "LayerChannels[layer]: " << layerChannels[layer] << endl;
			cout << "Channel: " << channel << endl;
			hRateTotChannelNorm[layer][channel] = (TH2F*) hRateTotChannel[layer][channel]->Clone(
				Form("hRateTotNorm%s_%d", layers[layer], channel)
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
				double xCenter = hRateTotNorm->GetXaxis()->GetBinCenter(rateBin);
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
