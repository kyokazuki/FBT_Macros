#include "printUPos.C"

void printUPosCoinedStart(const DataFBTCoin& inData) {
	inData.tree->SetBranchStatus("coin", 1);

	printUPosStart(inData);
}

void printUPosCoinedLoop(const DataFBTCoin& inData, Double_t targetDistance, Bool_t adjustAngle) {
	// check if at least one of the TOGAXSI coins is 1
	Int_t coinOnes = 0;
	for (UInt_t i = 1; i < 5; i++) {
		coinOnes += inData.coin[i];
	}
	if (coinOnes == 0) {
		return;
	}
	
	printUPosLoop(inData, targetDistance, adjustAngle);
}

void printUPosCoinedEnd(const DataBase& inData, Double_t targetDistance, Int_t gapPitch, const TString& outName) {
	printUPosEnd(inData, targetDistance, gapPitch, outName);
}

void printUPosCoined(const vector<TString>& inPaths, Double_t targetDistance, Int_t gapPitch, Bool_t adjustAngle = 1) {
	// load data
	DataFBTCoin inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printUPosCoinedStart(inData);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printUPosCoinedLoop(inData, targetDistance, adjustAngle);
	}

	TString outName;
	if (adjustAngle) {
		outName = Form("%s_uPosCoin", inData.runNum.Data());
	} else {
		outName = Form("%s_uPosCoin_unadjusted", inData.runNum.Data());
	}
	printUPosCoinedEnd(inData, targetDistance, gapPitch, outName);
}

