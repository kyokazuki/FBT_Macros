#include <TString.h>

#include "analyzeGrouped.C"

void analyzeGroupedScaled(const TString& inPath, const Float_t (&totRange)[2] = {0.5, 10}) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("timeGate", 1);
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYERS[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYERS[layer]), 1);
	}

	analyzeGroupedStart(totRange);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeGroupedLoop(inData, totRange);
	}

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeGroupedScaled.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
	);
	analyzeGroupedEnd(inData, totRange, graphPath);
}

