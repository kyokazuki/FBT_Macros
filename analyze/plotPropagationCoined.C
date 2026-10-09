#include "plotPropagation.C"

void plotPropagationCoinedStart(
	const DataFBTCoin& inData
) {
	inData.tree->SetBranchStatus("coin", 1);
	
	plotPropagationStart(inData);
}

void plotPropagationCoinedLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2]
) {
	// if (inData.coin[0] != 0) {
	// if (!(inData.coin[0]==1 && inData.coin[1]==0 && inData.coin[2]==0 && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0)) {
	if (!((inData.coin[1]==1 || inData.coin[2]==1 || inData.coin[3]==1 || inData.coin[4]==1 || inData.coin[5]==1) && inData.coin[6]==0)) {
	// if (!(inData.coin[0]==0 && (inData.coin[1]==1 || inData.coin[2]==1) && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0)) {
		return;
	}

	plotPropagationLoop(inData, totRange);
}

void plotPropagationCoinedEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const TString& outName
) {
	plotPropagationEnd(inData, totRange, outName);
}

void plotPropagationCoined(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {0.5, 10.0}
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotPropagationCoinedStart(inData);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotPropagationCoinedLoop(inData, totRange);
	}

	TString outName = Form("%s_propagationCoined", inData.runNum.Data());
	plotPropagationCoinedEnd(inData, totRange, outName);
}

