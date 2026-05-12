#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH2F.h>
#include <TF2.h>
#include <TChain.h>
#include <TGraph.h>
#include <TSystem.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include "utils/loadData.C"
#include "utils/printProgress.C"

const Float_t totRange[2] = {20e3, 100e3};

const Float_t posRange[2] = {-300.5, 299.5};
const Float_t posBins = 600;
const Int_t totBins = 500;

TH2F* hPos = nullptr;
TH2F* hPosAligned = nullptr;
TH2F* hPosAlignedDt = nullptr;
TH2F* hPosAlignedTotX = nullptr;
TH2F* hPosAlignedTotY = nullptr;
TH2F* hPosAlignedTotU = nullptr;
TH2F* hPosAlignedGateX = nullptr;
TH2F* hPosAlignedGateY = nullptr;
TH2F* hPosAlignedGateU = nullptr;

void plotPosition(const vector<TString>& inPath) {
	DataFBT2 inData(inPath, "events");

	inData.tree->SetBranchStatus("*", 0);
	inData.tree->SetBranchStatus("timeGate", 1);
	const char* layers[3] = {"X","Y","U"};
	for (size_t i = 0; i < 3; i++) {
		inData.tree->SetBranchStatus(Form("time%s", layers[i]), 1);
		inData.tree->SetBranchStatus(Form("tot%s", layers[i]), 1);
		inData.tree->SetBranchStatus(Form("xi%s", layers[i]), 1);
	}

	delete hPos;
	delete hPosAligned;
	delete hPosAlignedDt;
	delete hPosAlignedTotX;
	delete hPosAlignedTotY;
	delete hPosAlignedTotU;
	delete hPosAlignedGateX;
	delete hPosAlignedGateY;
	delete hPosAlignedGateU;

	hPos = new TH2F("hPos","hPos", posBins, -0.5, 599.5, 320, -0.5, 319.5);
	hPosAligned = new TH2F("hPosAligned","hPosAligned", posBins, posRange[0], posRange[1], 320, -0.5, 319.5);
	hPosAlignedDt = new TH2F("hPosAlignedDt","hPosAlignedDt", posBins, posRange[0], posRange[1], 100, 0, 50e3);
	hPosAlignedTotX = new TH2F("hPosAlignedTotX","hPosAlignedTotX", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedTotY = new TH2F("hPosAlignedTotY","hPosAlignedTotY", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedTotU = new TH2F("hPosAlignedTotU","hPosAlignedTotU", posBins, posRange[0], posRange[1], totBins, totRange[0], totRange[1]);
	hPosAlignedGateX = new TH2F("hPosAlignedGateX","hPosAlignedGateX", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	hPosAlignedGateY = new TH2F("hPosAlignedGateY","hPosAlignedGateY", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);
	hPosAlignedGateU = new TH2F("hPosAlignedGateU","hPosAlignedGateU", posBins, posRange[0], posRange[1], 100, -3880000, -3840000);

	// loop through all inData.tree
	Long64_t nskipped = 0;
	Long64_t pos = 0;
	Long64_t posAligned = 0;
	Long64_t dt = 0;

	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);

		inData.tree->GetEntry(entry);

		// xiX contains the gate entry so has to be > 1
		if (!(inData.xiV[0]->size() > 0 && inData.xiV[1]->size() > 0 && inData.xiV[2]->size() > 0)) {
			nskipped++;
			continue;
		}
		// if (!((*inData.timeV[0])[0] < (*inData.timeV[1])[0] && (*inData.timeV[1])[0] < (*inData.timeV[2])[0])) {
		// 	nskipped++;
		// 	continue;
		// }
		if (!(
			(*inData.totV[0])[0] >= totRange[0] && (*inData.totV[0])[0] < totRange[1] &&
			(*inData.totV[1])[0] >= totRange[0] && (*inData.totV[1])[0] < totRange[1] &&
			(*inData.totV[2])[0] >= totRange[0] && (*inData.totV[2])[0] < totRange[1]
		)) {
			nskipped++;
			continue;
		}

		pos = (*inData.xiV[0])[0] + (*inData.xiV[1])[0];
		// posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] + 39.14285)/0.73142;
		posAligned = (*inData.xiV[0])[0] + (*inData.xiV[1])[0] - ((*inData.xiV[2])[0] + 70)/0.83; // FDC1 test cosmic ray
		dt = abs((*inData.timeV[0])[0] - (*inData.timeV[1])[0]) + abs((*inData.timeV[1])[0] - (*inData.timeV[2])[0]) + abs((*inData.timeV[2])[0] - (*inData.timeV[0])[0]);

		hPos->Fill(pos, (*inData.xiV[2])[0]);
		hPosAligned->Fill(posAligned, (*inData.xiV[2])[0]);
		hPosAlignedDt->Fill(posAligned, dt);
		hPosAlignedTotX->Fill(posAligned, (*inData.totV[0])[0]);
		hPosAlignedTotY->Fill(posAligned, (*inData.totV[1])[0]);
		hPosAlignedTotU->Fill(posAligned, (*inData.totV[2])[0]);
		hPosAlignedGateX->Fill(posAligned, (*inData.timeV[0])[0] - (*inData.timeGate)[0]);
		hPosAlignedGateY->Fill(posAligned, (*inData.timeV[1])[0] - (*inData.timeGate)[0]);
		hPosAlignedGateU->Fill(posAligned, (*inData.timeV[2])[0] - (*inData.timeGate)[0]);
	}

	hPos->SetTitle(";xiX+xiY;xiU");
	hPosAligned->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;xiU");
	hPosAlignedDt->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;sum of all fiber event time differences");
	hPosAlignedTotX->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;totX");
	hPosAlignedTotY->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;totY");
	hPosAlignedTotU->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;totU");
	hPosAlignedGateX->SetTitle(";(xiX+xiY)-(xiU+30.4)/0.7;timeX-timeGate");
	hPosAlignedGateY->SetTitle(";(xiY+xiY)-(xiU+30.4)/0.7;timeY-timeGate");
	hPosAlignedGateU->SetTitle(";(xiU+xiY)-(xiU+30.4)/0.7;timeU-timeGate");

	cout << "Skipped " << nskipped << "/" << inData.entries << endl;
}
