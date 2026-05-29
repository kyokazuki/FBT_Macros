#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <ROOT/RDataFrame.hxx>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <memory>

void processFriend(const TString& inPath1, const TString& inPath2) {
	TString runNumber = TString(gSystem->BaseName(inPath1))(0,4);

	TFile inFile1(inPath1);
	TFile inFile2(inPath2);

	auto inTree1 = (TTree*)inFile1.Get("events");
	auto inTree2 = (TTree*)inFile2.Get("tree");

	inTree1->AddFriend(inTree2);

	ROOT::RDataFrame df(*inTree1);

	cout << "Saving file..." << endl;
	df.Snapshot("tree", Form("%s_friended.root", runNumber.Data()));
}

void processFriends(const vector<TString>& inPaths, const vector<TString>& treeNames) {
    TString runNumber = TString(gSystem->BaseName(inPaths[0]))(0,4);

    vector<unique_ptr<TFile>> inFiles;
    vector<TTree*> inTrees;

    for (size_t i = 0; i < inPaths.size(); ++i) {
        inFiles.emplace_back(TFile::Open(inPaths[i]));

        TTree* tree = dynamic_cast<TTree*>(inFiles.back()->Get(treeNames[i]));

        inTrees.push_back(tree);

        if (i > 0) {
            inTrees[0]->AddFriend(inTrees[i]);
		}
	}

	ROOT::RDataFrame df(*inTrees[0]);

	cout << "Saving file..." << endl;
	df.Snapshot("tree", Form("%s_friended.root", runNumber.Data()));
}
