#include "plotTotZ.C"

void plotTotZCoinedStart(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2]
) {
	inData.tree->SetBranchStatus("coin", 1);
	
	plotTotZStart(inData, totRange);
}

void plotTotZCoinedLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2]
) {
	if (!(inData.coin[0]==1 && inData.coin[1]==0 && inData.coin[2]==0 && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0)) {
		return;
	}

	plotTotZLoop(inData, totRange);
}

void plotTotZCoinedEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const TString& outName
) {
	plotTotZEnd(inData, totRange, outName);
}

void plotTotZCoined(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2]
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotTotZCoinedStart(inData, totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotTotZCoinedLoop(inData, totRange);
	}

	TString outName = Form("%s_totZCoined", inData.runNum.Data());
	plotTotZCoinedEnd(inData, totRange, outName);
}

