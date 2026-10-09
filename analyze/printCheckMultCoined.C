#include "printCheckMult.C"

void printCheckMultCoinedStart(
	const DataFBTCoin& inData,
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2]
) {
	inData.tree->SetBranchStatus("coin", 1);
	
	printCheckMultStart(inData, totRange1, totRange2, totRange);
}

void printCheckMultCoinedLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2]
) {
	// if (!(inData.coin[0] == 1)) {
	// if (!((inData.coin[1]==1 || inData.coin[2]==1 || inData.coin[3]==1 || inData.coin[4]==1 || inData.coin[5]==1) && inData.coin[6]==0)) {
	// if (!(inData.coin[0]==1 && inData.coin[1]==0 && inData.coin[2]==0 && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0)) {
	if (!(inData.coin[0]==0 && inData.coin[1]==0 && inData.coin[2]==0 && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==1)) {
		return;
	}

	printCheckMultLoop(inData, totRange1, totRange2, totRange);
}

void printCheckMultCoinedEnd(
	const DataFBTCoin& inData,
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2],
	const TString& outName
) {
	printCheckMultEnd(inData, totRange1, totRange2, totRange, outName);
}

void printCheckMultCoined(
	const vector<TString>& inPaths, 
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2] = {30e3, 1e6}
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printCheckMultCoinedStart(inData, totRange1, totRange2, totRange);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printCheckMultCoinedLoop(inData, totRange1, totRange2, totRange);
	}

	TString outName = Form("%s_checkMultCoined", inData.runNum.Data());
	printCheckMultCoinedEnd(inData, totRange1, totRange2, totRange, outName);
}

