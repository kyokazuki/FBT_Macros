#include <TH1F.h>

#include <TSystem.h>
#include <TCanvas.h>
#include <TStyle.h>

#include <iostream>
#include <stdlib.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

// initialize graphs
vector<TH1F*> hMult(3);

void plotMultHodoStart(const DataFBTHodo& inData) {
	inData.tree->SetBranchStatus("totX", 1);
	inData.tree->SetBranchStatus("totY", 1);
	inData.tree->SetBranchStatus("totU", 1);
	inData.tree->SetBranchStatus("coin", 1);
	inData.tree->SetBranchStatus("fQCal", 1);
	inData.tree->SetBranchStatus("fID", 1);

	for (Int_t layer = 0; layer < 3; layer++) {
		hMult[layer] = new TH1F(
			Form("hMult%c", LAYERS[layer]),
			"",
			100, -0.5, 99.5
		);
	}
}

void plotMultHodoLoop(const DataFBTHodo& inData, const vector<Float_t>& totRange, Int_t id, const vector<Double_t>& qRange) {
	if (!(inData.coin[0] == 1 && inData.fQCal[id] >= qRange[0] && inData.fQCal[id] <= qRange[1])) {
		return;
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		Int_t mult = 0;
		for (UInt_t hit = 0; hit < inData.totV[layer]->size(); hit++) {
			if ((*inData.totV[layer])[hit] >= totRange[0] && (*inData.totV[layer])[hit] <= totRange[1]) {
				mult += 1;
			}
		}
		hMult[layer]->Fill(mult);
	}
}

void plotMultHodoEnd(const vector<Float_t>& totRange, Int_t id, const vector<Double_t>& qRange) {
	for (Int_t layer = 0; layer < 3; layer++) {
		Long64_t total = hMult[layer]->GetEntries();
		Float_t effcy = (total - hMult[layer]->GetBinContent(1)) / total;
		hMult[layer]->SetTitle(Form(
			"mult%c (tot={%.0e, %.0e}, id=%d, q={%.0e, %.0e}) (effcy=%.4f);multiplicity", 
			LAYERS[layer], totRange[0], totRange[1], id, qRange[0], qRange[1], effcy
		));
		zoomAxisX(hMult[layer], 0, 2);
	}
}

void plotMultHodo(const TString& inPath, const vector<Float_t>& totRange, Int_t id, const vector<Double_t>& qRange) {
	TString runNumber = TString(gSystem->BaseName(inPath))(0,4);
	TString graphPath = Form(
        "%s/%s_analyzeMultHodo.pdf",
        gSystem->DirName(inPath),
        runNumber.Data()
	);

	// Set up variables to read from inData.tree
	DataFBTHodo inData({inPath}, "tree");
	inData.tree->SetBranchStatus("*", 0);

	plotMultHodoStart(inData);

	// loop through all events
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		plotMultHodoLoop(inData, totRange, id, qRange);
	}

	plotMultHodoEnd(totRange, id, qRange);
}

