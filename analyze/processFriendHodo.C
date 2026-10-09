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

void processFriendHodo(const TString& inPath1, const TString& inPath2) {
	gROOT->SetBatch(kTRUE);

	DataFBT2 inData1({inPath1}, "events");
	DataHodo inData2({inPath2}, "tree");
	inData2.tree->SetBranchStatus("*", 0);
	inData2.tree->SetBranchStatus("coin", 1);
	inData2.tree->SetBranchStatus("fID", 1);
	inData2.tree->SetBranchStatus("fQCal", 1);
	inData2.tree->SetBranchStatus("fTUCal", 1);
	inData2.tree->SetBranchStatus("fTDCal", 1);

	cout << "Input File 1: " << inData1.entries << " entries" << endl;
	cout << "Input File 2: " << inData2.entries << " entries" << endl;
	if (inData1.entries != inData2.entries) {
		cout << "Enties don't match!" << endl;
		return;
	}
	
	TFile* outFile = createOutFile(inPath1, "_hodo.root");
	TTree* outTree = inData1.tree->CloneTree(0);
	outTree->Branch("coin", &inData2.coin, "coin[16]/O");
	outTree->Branch("fID", &inData2.fID, "fID[40]/I");
	outTree->Branch("fQCal", &inData2.fQCal, "fQCal[40]/D");
	outTree->Branch("fTUCal", &inData2.fTUCal, "fTUCal[40]/D");
	outTree->Branch("fTDCal", &inData2.fTDCal, "fTDCal[40]/D");

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
