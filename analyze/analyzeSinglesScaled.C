#include <TString.h>

#include "analyzeSingles.C"

void analyzeSinglesScaled(const TString& inPath, Bool_t ext = 1) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	analyzeSinglesStart(MAX_TOT_RANGE_SCALED);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeSinglesLoop(inData, entry, MAX_TOT_RANGE_SCALED);
	}

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeSinglesScaled.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
    );
	analyzeSinglesEnd(inData, graphPath, ext);
}

