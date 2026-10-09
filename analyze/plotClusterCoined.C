#include "plotCluster.C"

void plotClusterCoinedStart(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2]
) {
	inData.tree->SetBranchStatus("coin", 1);
	
	plotClusterStart(inData, totRange);
}

void plotClusterCoinedLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2]
) {
	// if (!(inData.coin[0]==0 && (inData.coin[1]==1 || inData.coin[2]==1) && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0)) {
	if (!(inData.coin[0]==0 && inData.coin[1]==1 && inData.coin[2]==0 && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0)) {
		return;
	}

	plotClusterLoop(inData, totRange);
}

void plotClusterCoinedEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const TString& outName,
	Bool_t saveGraph
) {
	plotClusterEnd(inData, totRange, outName, saveGraph);
}

void plotClusterCoined(
	const vector<TString>& inPaths,
	const Float_t (&totRange)[2] = {30e3, 160e3},
	Bool_t saveGraph = 1
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	plotClusterCoinedStart(inData, totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotClusterCoinedLoop(inData, totRange);
	}

	TString outName = Form("%s_clusterCoined", inData.runNum.Data());
	plotClusterCoinedEnd(inData, totRange, outName, saveGraph);
}

