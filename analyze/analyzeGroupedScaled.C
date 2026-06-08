#include <TString.h>

#include "analyzeGrouped.C"

void analyzeGroupedScaled(
	const TString& inPath, 
	const Float_t (&totRange)[2]		= {50e3, 1e6},
	const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE,
	const Float_t (&posRange)[2]		= MAX_POS_RANGE
) {
	DataFBT2 inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);

	analyzeGroupedStart(inData, totRange);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeGroupedLoop(inData, totRange, timingRange, posRange);
	}

	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeGroupedScaled.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
	);
	analyzeGroupedEnd(inData, totRange, timingRange, posRange, graphPath);
}

