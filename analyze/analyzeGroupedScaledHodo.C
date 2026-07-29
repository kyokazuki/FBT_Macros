#include <TString.h>

#include "analyzeGroupedHodo.C"

void analyzeGroupedScaledHodo(
	const TString& inPath, 
	const Float_t (&totRange)[2]	= {0.5, 10},
	const Int_t (&idRange)[2]		= HODO_MAX_ID_RANGE, 
	const Double_t (&qRange)[2]		= HODO_MAX_Q_RANGE, 
	const vector<Int_t>& coins		= ALL_COINS
) {
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);

	analyzeGroupedHodoStart(inData, totRange);

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		analyzeGroupedHodoLoop(inData, totRange, idRange, qRange, coins);
	}

	TString graphPath = Form(
        "%s/%s_analyzeGroupedScaledHodo.pdf",
        gSystem->DirName(inPath),
		inData.runNum.Data()
	);
	analyzeGroupedHodoEnd(inData, totRange, idRange, qRange, coins, graphPath);
}

