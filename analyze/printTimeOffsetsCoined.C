#include "printTimeOffsets.C"

void printTimeOffsetsCoinedStart(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	inData.tree->SetBranchStatus("coin", 1);

	printTimeOffsetsStart(inData, totRange, timingRange);
}

void printTimeOffsetsCoinedLoop(
	const DataFBTCoin& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2]
) {
	// if (!((inData.coin[1]==1 || inData.coin[2]==1 || inData.coin[3]==1 || inData.coin[4]==1 || inData.coin[5]==1) && inData.coin[6]==0)) {
	if (!(inData.coin[0]==0 && (inData.coin[1]==1 || inData.coin[2]==1) && inData.coin[3]==0 && inData.coin[4]==0 && inData.coin[5]==0 && inData.coin[6]==0)) {
	// if (!(inData.coin[0]==0 && (inData.coin[1]==1 || inData.coin[2]==1 || inData.coin[3]==1 || inData.coin[4]==1 || inData.coin[6]==1) && inData.coin[5]==0)) {
		return;
	}

	printTimeOffsetsLoop(inData, totRange, timingRange);
}

void printTimeOffsetsCoinedEnd(
	const DataBase& inData,
	const Float_t (&totRange)[2],
	const Long64_t (&timingRange)[2],
	const TString& outName
) {
	printTimeOffsetsEnd(inData, totRange, timingRange, outName);
}

void printTimeOffsetsCoined(
	const vector<TString>& inPaths, 
	const Float_t (&totRange)[2] = {30e3, 1e6},
	// const Long64_t (&timingRange)[2] = {-3540000, -3490000}
	const Long64_t (&timingRange)[2] = {-10000, 35000}
) {
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printTimeOffsetsCoinedStart(inData, totRange, timingRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printTimeOffsetsCoinedLoop(inData, totRange, timingRange);
	}

	TString outName = Form("%s_timeOffsetsCoined", inData.runNum.Data());
	printTimeOffsetsCoinedEnd(inData, totRange, timingRange, outName);
}
