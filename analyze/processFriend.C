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
	const TString& treeName1, 
	const TString& treeName2
) {
	TString runNumber = TString(gSystem->BaseName(inPath1))(0,4);

	TFile inFile1(inPath1);
	TFile inFile2(inPath2);

	auto inTree1 = (TTree*)inFile1.Get(treeName1);
	auto inTree2 = (TTree*)inFile2.Get(treeName2);

	inTree1->AddFriend(inTree2);

	ROOT::RDataFrame df(*inTree1);

	cout << "Saving file..." << endl;
	df.Snapshot("tree", Form("%s_friended.root", runNumber.Data()));
}

