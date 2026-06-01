#include <TString.h>

#include "analyzeSingles.C"

void analyzeSinglesScaled(
	const TString& inPath, 
	Bool_t ext = 1, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE_SCALED
) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("time", 1);
	inData.tree->SetBranchStatus("channelID", 1);
	inData.tree->SetBranchStatus("tot", 1);
	inData.tree->SetBranchStatus("energy", 1);
	inData.tree->SetBranchStatus("xi", 1);
	inData.tree->SetBranchStatus("yi", 1);

	analyzeSinglesStart(totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeSinglesLoop(inData, entry, totRange);
	}

	TString graphPath = Form(
        "%s/%s_analyzeSinglesScaled.pdf",
        gSystem->DirName(inPath),
        inData.runNum.Data()
    );
	analyzeSinglesEnd(inData, ext, totRange, graphPath);
}

