#include "plotTimeRes.C"

void plotTimeResCoinedStart(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2]
) {
	inData.tree->SetBranchStatus("coin", 1);
	
	plotTimeResStart(inData, totRange);
}

void plotTimeResCoinedLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2]
) {
	if (inData.coin[0] != 1) {
		return;
	}

	plotTimeResLoop(inData, totRange);
}

void plotTimeResCoinedEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const TString& outName,
	Bool_t saveGraph
) {
	plotTimeResEnd(inData, totRange, outName, saveGraph);
}

void plotTimeResCoined(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {0.5, 10},
	Bool_t saveGraph = 1
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotTimeResCoinedStart(inData, totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTimeResCoinedLoop(inData, totRange);
	}

	TString outName = Form("%s_timeResCoined", inData.runNum.Data());
	plotTimeResCoinedEnd(inData, totRange, outName, saveGraph);
}

