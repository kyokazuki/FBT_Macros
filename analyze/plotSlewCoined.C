#include "plotSlew.C"

void plotSlewCoinedStart(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	inData.tree->SetBranchStatus("coin", 1);
	
	plotSlewStart(inData, totRange, timingRange);
}

void plotSlewCoinedLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	if (inData.coin[0] != 1) {
		return;
	}

	plotSlewLoop(inData, totRange, timingRange);
}

void plotSlewCoinedEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const TString& outName,
	Bool_t saveGraph
) {
	plotSlewEnd(inData, totRange, timingRange, outName, saveGraph);
}

void plotSlewCoined(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {0.5, 10.0},
	const Long64_t (&timingRange)[2] = {-10000, 20000},
	Bool_t saveGraph = 1
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotSlewCoinedStart(inData, totRange, timingRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotSlewCoinedLoop(inData, totRange, timingRange);
	}

	TString outName = Form("%s_slewCoined", inData.runNum.Data());
	plotSlewCoinedEnd(inData, totRange, timingRange, outName, saveGraph);
}

