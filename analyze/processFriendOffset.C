#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

#include "utils/printProgress.C"

void processFriendOffset(const TString& inputPath1, const TString& inputPath2, const vector <Int_t>& offsets) {
	gROOT->SetBatch(kTRUE);

	TFile* inputFile1 = TFile::Open(inputPath1);
	TFile* inputFile2 = TFile::Open(inputPath2);
	TTree* inputTree1 = (TTree*)inputFile1->Get("events");
	TTree* inputTree2 = (TTree*)inputFile2->Get("mtree");
	TTree* inputTree3 = (TTree*)inputFile2->Get("stree_offset");
	TString runNumber = TString(gSystem->BaseName(inputPath1))(0,4);

	Long64_t events1 = inputTree1->GetEntries();
	Long64_t events2 = inputTree2->GetEntries();
	cout << "Input File 1: " << events1 << " entries" << endl;
	cout << "Input File 2: " << events2 << " entries" << endl;

	Double_t ult, urt, dlt, drt, ulq, urq, dlq, drq;
	inputTree2->SetBranchAddress("ult", &ult);
	inputTree2->SetBranchAddress("urt", &urt);
	inputTree2->SetBranchAddress("dlt", &dlt);
	inputTree2->SetBranchAddress("drt", &drt);
	inputTree2->SetBranchAddress("ulq", &ulq);
	inputTree2->SetBranchAddress("urq", &urq);
	inputTree2->SetBranchAddress("dlq", &dlq);
	inputTree2->SetBranchAddress("drq", &drq);
	Long64_t scaler;
	inputTree3->SetBranchAddress("scaler32", &scaler);
	
	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	TString graphPath = Form("%s_friendOffset_%d-%d.pdf", runNumber.Data(), offsets.front(), offsets.back());
	c1->Print(Form("%s[", graphPath.Data()));

	for (UInt_t i = 0; i < offsets.size(); i++) {
		TString output_path = inputPath1;
		output_path.ReplaceAll(".root", Form("_friended_offset%d.root", offsets[i]));
		TFile* outputFile = new TFile(output_path, "RECREATE");

		TTree* outputTree = inputTree1->CloneTree(0);
		outputTree->Branch("ult", &ult);
		outputTree->Branch("urt", &urt);
		outputTree->Branch("dlt", &dlt);
		outputTree->Branch("drt", &drt);
		outputTree->Branch("ulq", &ulq);
		outputTree->Branch("urq", &urq);
		outputTree->Branch("dlq", &dlq);
		outputTree->Branch("drq", &drq);
		outputTree->Branch("scaler32", &scaler);

		if (events1 == events2) {
			for (Long64_t event = 0; event < events1; event++) {
				printProgress(event, events1);

				inputTree1->GetEntry(event);
				inputTree2->GetEntry(event + offsets[i]);
				outputTree->Fill();
			}
		} else if (events1 < events2) {
			for (Long64_t event = 0; event < events1; event++) {
				printProgress(event, events1);

				inputTree1->GetEntry(event);
				inputTree2->GetEntry(event + offsets[i]);
				outputTree->Fill();
			}
		} else if (events1 > events2) {
			for (Long64_t event = 0; event < events2; event++) {
				printProgress(event, events2);

				inputTree1->GetEntry(event + offsets[i]);
				inputTree2->GetEntry(event);
				outputTree->Fill();
			}
		}

		outputTree->Draw("totX:pow(ulq*urq*dlq*drq,0.25)", "", "AP");
		TGraph* gr = (TGraph*)gPad->GetPrimitive("Graph");
		gr->GetXaxis()->SetLimits(0, 8000);
		gr->SetMinimum(0);
		gr->SetMaximum(600000);
		gr->SetTitle(Form("offset=%d", offsets[i]));
		c1->Print(graphPath.Data());

		outputFile->cd();
		outputTree->Write();
		outputTree->Reset();
		outputFile->Close();
	}
	c1->Print(Form("%s]", graphPath.Data()));
	inputFile1->Close();
	inputFile2->Close();
}
