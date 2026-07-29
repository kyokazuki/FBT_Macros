#include <vector>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>

#include "utils/createOutFile.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"

void processFriendCoin(const TString& inPath1, const TString& inPath2) {
	gROOT->SetBatch(kTRUE);

	DataFBT2 inData1({inPath1}, "events");
	DataCoin inData2({inPath2}, "tree");
	inData2.tree->SetBranchStatus("*", 0);
	inData2.tree->SetBranchStatus("coin", 1);

	cout << "Input File 1: " << inData1.entries << " entries" << endl;
	cout << "Input File 2: " << inData2.entries << " entries" << endl;
	if (inData1.entries != inData2.entries) {
		cout << "Enties don't match!" << endl;
		return;
	}
	
	TFile* outFile = createOutFile(inPath1, "_coined.root");
	TTree* outTree = inData1.tree->CloneTree(0);
	outTree->Branch("coin", &inData2.coin, "coin[16]/O");

	for (Long64_t entry = 0; entry < inData1.entries; entry++) {
		printProgress(entry, inData1.entries);
		inData1.tree->GetEntry(entry);
		inData2.tree->GetEntry(entry);

		outTree->Fill();
	}

	outTree->Write();
	outFile->Close();
	
	return;
}
