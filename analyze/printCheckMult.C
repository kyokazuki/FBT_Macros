#include <iostream>
#include <stdlib.h>

#include <TH1F.h>
#include <TCanvas.h>

#include "utils/addStats.C"
#include "utils/loadData.C"
#include "utils/printProgress.C"
#include "utils/zoomAxis.C"

vector<TH1F*> hMult1(3, nullptr);
vector<TH1F*> hMult2(3, nullptr);

void printCheckMultStart(
	const DataFBT2& inData,
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
	}

	for (Int_t layer = 0; layer < 3; layer++) {
		delete hMult1[layer];
		hMult1[layer] = new TH1F(
			Form("hMult1%c", LAYER_NAMES[layer]),
			Form(
				"multiplicity %c (max tot = {%.2g, %.2g});multiplicity", 
				LAYER_NAMES[layer],
				totRange1[0],
				totRange1[1]
			),
			100, 0.5, 100.5
		);

		delete hMult2[layer];
		hMult2[layer] = new TH1F(
			Form("hMult2%c", LAYER_NAMES[layer]),
			Form(
				"multiplicity %c (max tot = {%.2g, %.2g});multiplicity", 
				LAYER_NAMES[layer],
				totRange2[0],
				totRange2[1]
			),
			100, 0.5, 100.5
		);
	}
}

void printCheckMultLoop(
	const DataFBT2& inData,
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2]
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.totV[layer]->size() == 0) {
			return;
		}

		Int_t zNum = -1;
		if (inRange((*inData.totV[layer])[0], totRange1)) {
			zNum = 1;
		} else if (inRange((*inData.totV[layer])[0], totRange2)) {
			zNum = 2;
		} else {
			continue;
		}

		Int_t mult = 1;
		for (UInt_t hit = 1; hit < inData.totV[layer]->size(); hit++) {
			if (!inRange((*inData.totV[layer])[hit], totRange)) {
				continue;
			}
			mult++;
		}

		if (zNum == 1) {
			hMult1[layer]->Fill(mult);
		} else if (zNum == 2) {
			hMult2[layer]->Fill(mult);
		}
	}
}

void printCheckMultEnd(
	const DataFBT2& inData,
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2],
	const TString& outName
) {
	TCanvas* c1 = nullptr;
	c1 = new TCanvas("c1", "c1", 1500, 800);
	TString graphPath = Form("%s.pdf", outName.Data());
	c1->Print(Form("%s[", graphPath.Data()));
	c1->Divide(3, 2);
	Int_t cell = 1;

	for (Int_t layer = 0; layer < 3; layer++) {
		c1->cd(cell);
		cell++;
		gPad->SetGrid();
		gPad->SetLogy();

		Double_t totalEntries = hMult1[layer]->GetEntries();
		Double_t singleHitEntries = hMult1[layer]->GetBinContent(1);
		addStats(hMult1[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", totalEntries), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("single-hits = %.3f\%%", singleHitEntries / totalEntries * 100)
		});
		hMult1[layer]->Draw();
	}
	for (Int_t layer = 0; layer < 3; layer++) {
		c1->cd(cell);
		cell++;
		gPad->SetGrid();
		gPad->SetLogy();

		Double_t totalEntries = hMult2[layer]->GetEntries();
		Double_t singleHitEntries = hMult2[layer]->GetBinContent(1);
		addStats(hMult2[layer], {
			Form("run%s", inData.runNum.Data()),
			Form("entries = %.0f", totalEntries), 
			Form("tot = {%.3g, %.3g}", totRange[0], totRange[1]),
			Form("single-hits = %.3f\%%", singleHitEntries / totalEntries * 100)
		});
		hMult2[layer]->Draw();
	}

	c1->Print(graphPath);
	c1->Print(Form("%s]", graphPath.Data()));
	delete c1;
}

void printCheckMult(
	const vector<TString>& inPaths, 
	const Float_t (&totRange1)[2],
	const Float_t (&totRange2)[2],
	const Float_t (&totRange)[2] = {30e3, 1e6}
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	printCheckMultStart(inData, totRange1, totRange2, totRange);

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		printCheckMultLoop(inData, totRange1, totRange2, totRange);
	}

	TString outName = Form("%s_checkMult", inData.runNum.Data());
	printCheckMultEnd(inData, totRange1, totRange2, totRange, outName);
}
