#include "processShift.C"

void processShiftCoinedStart(DataFBTCoin& inData, const char* inName) {
	outTreeShifted->Branch("coin", &(*inData.coin), "coin[16]/O");

	processShiftStart(inData, inName);
}

void processShiftCoinedLoop(
	const DataFBTCoin& inData,
	const Long64_t (&timingRange)[2]
) {
	processShiftLoop(inData, timingRange);
}

void processShiftCoinedEnd() {
	processShiftEnd();
}

void processShiftCoined(
	const TString& inPath, 
	const char* inName,
	const Long64_t (&timingRange)[2] = {-14000, 34000}
) {
	cout << "Shifting timestamps for " << inPath << " with " << inName << "[X-U].tsv" << endl;

	// load trees
	DataFBTCoin inData({inPath}, "events");
	inData.tree->SetBranchStatus("*", 0);
	outFileShifted = createOutFile(inPath, "_shifted.root");
	outTreeShifted = new TTree("events", "outTree");

	processShiftCoinedStart(inData, inName);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		processShiftCoinedLoop(inData, timingRange);
	}

	processShiftCoinedEnd();
}

