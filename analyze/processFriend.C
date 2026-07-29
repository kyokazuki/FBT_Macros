#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <ROOT/RDataFrame.hxx>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <memory>

void processFriend(
	const TString& inPath1,
	const TString& inPath2,
	const TString& treeName1 = "events", 
	const TString& treeName2 = "tree", 
	const vector<TString>& branches = {"*"},
	const TString& outTreeName = "events"
) {
	TString runNumber = TString(gSystem->BaseName(inPath1))(0,4);

	TFile inFile1(inPath1);
	TFile inFile2(inPath2);

	auto inTree1 = (TTree*)inFile1.Get(treeName1);
	auto inTree2 = (TTree*)inFile2.Get(treeName2);

	if (inTree1->GetEntries() != inTree2->GetEntries()) {
		throw std::runtime_error("Trees have different numbers of entries!");
	}

	inTree2->SetBranchStatus("*", 0);
	for (TString branch : branches) {
		inTree2->SetBranchStatus(branch, 1);
	}

	// inTree1->AddFriend(inTree2->CloneTree());
	inTree1->AddFriend(inTree2);

	ROOT::RDataFrame df(*inTree1);

	cout << "Saving file..." << endl;
	df.Snapshot(outTreeName, Form("%s_friended.root", runNumber.Data()));
}

