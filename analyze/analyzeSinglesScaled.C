#include <TString.h>

#include "analyzeSingles.C"

void analyzeSinglesScaled(
	const TString& inPath, 
	Bool_t ext = 1, 
	const Float_t (&totRange)[2] = MAX_TOT_RANGE_SCALED,
	const Long64_t (&timingRange)[2] = MAX_TIMING_RANGE
) {
	DataFBT1 inData({inPath}, "data");
	inData.tree->SetBranchStatus("*", 0);

	analyzeSinglesStart(inData, totRange, timingRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeSinglesLoop(inData, entry, totRange, timingRange);
	}

	TString graphPath = Form(
        "%s/%s_analyzeSinglesScaled.pdf",
        gSystem->DirName(inPath),
        inData.runNum.Data()
    );
	analyzeSinglesEnd(inData, ext, totRange, timingRange, graphPath);
}

