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

#include "utils/parameters.C"
#include "utils/loadData.C"
#include "utils/createOutFile.C"
#include "utils/printProgress.C"

void processFriendOffset(const TString& inPath1, const TString& inPath2, const vector <Int_t>& offsets) {
	gROOT->SetBatch(kTRUE);

	TString runNumber = TString(gSystem->BaseName(inPath1))(0,4);

	DataFBT2 inData1({inPath1}, "events");
	DataVME1 inData2({inPath2}, "mtree");

	cout << "Input File 1: " << inData1.entries << " entries" << endl;
	cout << "Input File 2: " << inData2.entries << " entries" << endl;
	
	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	TString graphPath = Form("%s_friendOffset_%d-%d.pdf", runNumber.Data(), offsets.front(), offsets.back());
	c1->Print(Form("%s[", graphPath.Data()));

	for (UInt_t i = 0; i < offsets.size(); i++) {
		TFile* outFile = createOutFile(inPath1, Form("_friended_offset%d.root", offsets[i]));
		TTree* outTree = inData1.tree->CloneTree(0);
		outTree->Branch("ult", &inData2.ult);
		outTree->Branch("urt", &inData2.urt);
		outTree->Branch("dlt", &inData2.dlt);
		outTree->Branch("drt", &inData2.drt);
		outTree->Branch("ulq", &inData2.ulq);
		outTree->Branch("urq", &inData2.urq);
		outTree->Branch("dlq", &inData2.dlq);
		outTree->Branch("drq", &inData2.drq);
		outTree->Branch("scaler32", &inData2.scaler);

		if (inData1.entries == inData2.entries) {
			for (Long64_t entry = 0; entry < inData1.entries; entry++) {
				printProgress(entry, inData1.entries);

				inData1.tree->GetEntry(entry);
				inData2.tree->GetEntry(entry + offsets[i]);
				outTree->Fill();
			}
		} else if (inData1.entries < inData2.entries) {
			for (Long64_t entry = 0; entry < inData1.entries; entry++) {
				printProgress(entry, inData1.entries);

				inData1.tree->GetEntry(entry);
				inData2.tree->GetEntry(entry + offsets[i]);
				outTree->Fill();
			}
		} else if (inData1.entries > inData2.entries) {
			for (Long64_t entry = 0; entry < inData2.entries; entry++) {
				printProgress(entry, inData2.entries);

				inData1.tree->GetEntry(entry + offsets[i]);
				inData2.tree->GetEntry(entry);
				outTree->Fill();
			}
		}

		outTree->Draw("totX:pow(ulq*urq*dlq*drq,0.25)", "", "AP");
		TGraph* gr = (TGraph*)gPad->GetPrimitive("Graph");
		gr->GetXaxis()->SetLimits(0, 8000);
		gr->SetMinimum(0);
		gr->SetMaximum(600000);
		gr->SetTitle(Form("offset=%d", offsets[i]));
		c1->Print(graphPath.Data());

		outTree->Write();
		outTree->Reset();
		outFile->Close();
	}
	c1->Print(Form("%s]", graphPath.Data()));
}
