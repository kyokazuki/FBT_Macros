#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TString.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include "utils/parameters.C"
#include "utils/loadData.C"
#include "utils/createOutFile.C"
#include "utils/printProgress.C"

int processRate(const TString& inPath) {
	cout << "Rating " << inPath << endl;

	const bool SAVE_GRAPH = 1;
	const Int_t windowHalfWidth = 100;
	const Int_t windowFullWidth = 2 * windowHalfWidth + 1;

	DataFBT4 inData({inPath}, "events");

	TFile* outFile = createOutFile(inPath, "_rated.root");
	TTree* outTree = inData.tree->CloneTree();

	// preload
	vector <Long64_t> timeGateEntries(inData.entries);
	vector <Int_t> scalerEntries(inData.entries);
	for (Long64_t i = 0; i < inData.entries; i++) {
		inData.tree->GetEntry(i);
		timeGateEntries[i]  = (*inData.timeGate)[0];
		scalerEntries[i] = inData.scaler[10];
	}

	// load branches for output tree
	Float_t rate;
	TBranch* rateBranch = outTree->Branch("rate", &rate);

	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("timeGate", 1);
	inData.tree->SetBranchStatus("scaler", 1);

	// fill rate banch
	for (Int_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		if (entry < windowHalfWidth || entry >= inData.entries - windowHalfWidth) {
			rate = 0.f;
		} else {
			Long64_t dt = timeGateEntries[entry + windowHalfWidth] - timeGateEntries[entry - windowHalfWidth];
			Int_t    dc = scalerEntries[entry + windowHalfWidth] - scalerEntries[entry - windowHalfWidth];
			rate = 1e12 * (Float_t)dc / (Float_t)dt;
		}
		rateBranch->Fill();
	}
	cout << endl;

	// save rate graph
	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	if (SAVE_GRAPH) {
		TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
		TString graphPath = Form("%s_rate.pdf", runNumber.Data());
		outTree->Draw("rate:timeGate", "Entry$ % 100 == 0");
		c1->Print(graphPath);
	}
	delete c1;

	// Save output file
	outTree->Write();
	outFile->Close();

	return 0;
}
