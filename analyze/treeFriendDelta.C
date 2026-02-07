#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>

#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>

TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
TGraph *gDdtime = nullptr;

int treeFriendDelta(const TString& inputPath1, const TString& inputPath2) {
	// Load input and output files
	if (gSystem->AccessPathName(inputPath1)) {
		cout << inputPath1 << " does not exist!" << endl;
		return 2;
	}
	if (gSystem->AccessPathName(inputPath2)) {
		cout << inputPath2 << " does not exist!" << endl;
		return 2;
	}
	TFile* inputFile1 = TFile::Open(inputPath1);
	TFile* inputFile2 = TFile::Open(inputPath2);
	TString outputPath = inputPath1;
	outputPath.ReplaceAll(".root", "_friended.root");
	TFile* outputFile = new TFile(outputPath, "RECREATE");
	outputFile->cd();
	cout << "Friending " << inputPath1 << " with " << inputPath2 << endl;

	// Load input and output trees
	TTree* inputTree1 = (TTree*)inputFile1->Get("events");
	TTree* inputTree2 = (TTree*)inputFile2->Get("tree");
	TTree* outputTree = inputTree1->CloneTree(0);
	// Load branches for input trees
	vector<Long64_t>* timeX = nullptr;
	vector<Long64_t>* timeY = nullptr;
	vector<Long64_t>* timeU = nullptr;
	vector<Long64_t>* energyX = nullptr;
	vector<Long64_t>* timeGate = nullptr;
	inputTree1->SetBranchAddress("timeX", &timeX);
	inputTree1->SetBranchAddress("timeY", &timeY);
	inputTree1->SetBranchAddress("timeU", &timeU);
	inputTree1->SetBranchAddress("energyX", &energyX);
	inputTree1->SetBranchAddress("timeGate", &timeGate);
	Double_t l1t, l2t, m1t, m2t, s1t, l1q, l2q, m1q, m2q, s1q, rft;
	Long64_t bbtime;
	Int_t scaler[32];
	inputTree2->SetBranchAddress("l1t", &l1t);
	inputTree2->SetBranchAddress("l2t", &l2t);
	inputTree2->SetBranchAddress("m1t", &m1t);
	inputTree2->SetBranchAddress("m2t", &m2t);
	inputTree2->SetBranchAddress("s1t", &s1t);
	inputTree2->SetBranchAddress("l1q", &l1q);
	inputTree2->SetBranchAddress("l2q", &l2q);
	inputTree2->SetBranchAddress("m1q", &m1q);
	inputTree2->SetBranchAddress("m2q", &m2q);
	inputTree2->SetBranchAddress("s1q", &s1q);
	inputTree2->SetBranchAddress("rft", &rft);
	inputTree2->SetBranchAddress("bbtime", &bbtime);
	inputTree2->SetBranchAddress("scaler", scaler);
	// Load branches for output tree
	outputTree->Branch("l1t", &l1t);
	outputTree->Branch("l2t", &l2t);
	outputTree->Branch("m1t", &m1t);
	outputTree->Branch("m2t", &m2t);
	outputTree->Branch("s1t", &s1t);
	outputTree->Branch("l1q", &l1q);
	outputTree->Branch("l2q", &l2q);
	outputTree->Branch("m1q", &m1q);
	outputTree->Branch("m2q", &m2q);
	outputTree->Branch("s1q", &s1q);
	outputTree->Branch("rft", &rft);
	outputTree->Branch("bbtime", &bbtime);
	outputTree->Branch("scaler", scaler);
	Long64_t dtime, dbbtime;
	outputTree->Branch("dtime", &dtime);
	outputTree->Branch("dbbtime", &dbbtime);

	// Get input trees' entry counts
	Long64_t entries1 = inputTree1->GetEntries();
	Long64_t entries2 = inputTree2->GetEntries();
	cout << "Input File 1: " << entries1 << " entries" << endl;
	cout << "Input File 2: " << entries2 << " entries" << endl;

	// Set appropriate time ratio and time resolution
	inputTree1->GetEntry(0);
	Long64_t firstTime1 = (*timeGate)[0];
	inputTree1->GetEntry(entries1 - 1);
	Long64_t lastTime1 = (*timeGate)[0];
	inputTree2->GetEntry(0);
	Long64_t firstTime2 = bbtime;
	inputTree2->GetEntry(entries2 - 1);
	Long64_t lastTime2 = bbtime;
	Double_t timeRatio = (Double_t)(lastTime1 - firstTime1) / (Double_t)(lastTime2 - firstTime2);
	cout << setprecision(10) << "Time ratio: (" << lastTime1 << "-" << firstTime1 << ")/(" << lastTime2 << "-" << firstTime2 << ") = " << timeRatio << endl;
	Long64_t timeRes = 50000;
	cout << "Time resolution: " << timeRes << endl;

	// Look for the first matching events
	cout << "Looking for the first matching events..." << endl;
	Long64_t entry1 = 0, entry2 = 0;
	Long64_t startEntry1 = -1, startEntry2 = -1;
	Long64_t startTime1 = 0, endTime1 = 0, dTime1 = 0;
	Long64_t startTime2 = 0, endTime2 = 0, dTime2 = 0;
	if (entries1 == entries2) {
		startEntry1 = 0;
		startEntry2 = 0;
		cout << "Event 0/" << entries1 - 1 << " in tree 1 matched with event 0/" << entries2 - 1 << " in tree 2" << endl;
	} else if (entries1 < entries2) {
		inputTree1->GetEntry(0);
		startTime1 = (*timeGate)[0];
		inputTree1->GetEntry(1);
		endTime1 = (*timeGate)[0];
		dTime1 = endTime1 - startTime1;

		for (entry2 = 0; entry2 <= entries2 - entries1; entry2++) {
			inputTree2->GetEntry(entry2);
			startTime2 = bbtime;
			inputTree2->GetEntry(entry2 + 1);
			endTime2 = bbtime;
			dTime2 = endTime2 - startTime2;

			if (dTime1 >= dTime2 * timeRatio - timeRes && dTime1 <= dTime2 * timeRatio + timeRes) {
				cout << "Event 0/" << entries1 - 1 << " in tree 1 matched with event " << entry2 << "/" << entries2 - 1 << " in tree 2" << endl;
				startEntry1 = 0;
				startEntry2 = entry2;
				break;
			}
		}
	} else if (entries1 > entries2) {
		inputTree2->GetEntry(0);
		startTime2 = bbtime;
		inputTree2->GetEntry(1);
		endTime2 = bbtime;
		dTime2 = endTime2 - startTime2;

		for (entry1 = 0; entry1 <= entries1 - entries2; entry1++) {
			inputTree1->GetEntry(entry1);
			startTime1 = (*timeGate)[0];
			inputTree1->GetEntry(entry1 + 1);
			endTime1 = (*timeGate)[0];
			dTime1 = endTime1 - startTime1;

			if (dTime1 >= dTime2 * timeRatio - timeRes && dTime1 <= dTime2 * timeRatio + timeRes) {
				cout << "Event " << entry1 << "/" << entries1 - 1 << " in tree 1 matched with event 0/" << entries2 - 1 << " in tree 2" << endl;
				startEntry1 = entry1;
				startEntry2 = 0;
				break;
			}
		}
	}
	if (startEntry1 == -1 && startEntry2 == -1) {
		cout << "Unable to match first event" << endl;
		return 2;
	}

	// Configure graph
	TString graphPath = inputPath1;
	graphPath.ReplaceAll(".root", "_friendDelta.pdf");
	c1->SetGridx(true);
	c1->SetGridy(true);
	TGraph *gDdtime = new TGraph();
	gDdtime->SetTitle("Ddtime;babi time[~ns];d(FBT time)-d(babi time)*timeRatio[ps]");
	gDdtime->GetXaxis()->SetLimits(firstTime2, lastTime2);
	// gDdtime->SetMaximum(timeRes);
	// gDdtime->SetMinimum(-1*timeRes);

	// Go through all entries and match events
	inputTree1->GetEntry(startEntry1);
	inputTree2->GetEntry(startEntry2);
	dtime = 0;
	dbbtime = 0;
	outputTree->Fill();
	startTime1 = (*timeGate)[0];
	startTime2 = bbtime;
	Long64_t skippedEntries1 = startEntry1;
	Long64_t skippedEntries2 = startEntry2;
	bool goNext1 = 1, goNext2 = 1;
	Long64_t graphPoint = 0;
	for (entry1 = startEntry1 + 1, entry2 = startEntry2 + 1; entry1 < entries1 && entry2 < entries2; entry1 += goNext1, entry2 += goNext2) {
		inputTree1->GetEntry(entry1);
		inputTree2->GetEntry(entry2);
		endTime1 = (*timeGate)[0];
		endTime2 = bbtime;
		dTime1 = endTime1 - startTime1;
		dTime2 = endTime2 - startTime2;

		gDdtime->SetPoint(graphPoint++, endTime2, dTime1 - dTime2 * timeRatio);

		if (dTime1 >= dTime2 * timeRatio - timeRes && dTime1 <= dTime2 * timeRatio + timeRes) {
			dtime = dTime1;
			dbbtime = dTime2;
			outputTree->Fill();
			startTime1 = (*timeGate)[0];
			startTime2 = bbtime;
			goNext1 = 1;
			goNext2 = 1;
		} else if (dTime1 < dTime2 * timeRatio - timeRes) {
			cout << "Skipping event " << entry1 << "/" << entries1 - 1 << " in tree 1 (" << dTime1 << ":" << dTime2 << ")" << endl;
			goNext1 = 1;
			goNext2 = 0;
			skippedEntries1 += 1;
		} else if (dTime1 > dTime2 * timeRatio + timeRes) {
			cout << "Skipping event " << entry2 << "/" << entries2 - 1 << " in tree 2 (" << dTime1 << ":" << dTime2 << ")" << endl;
			goNext1 = 0;
			goNext2 = 1;
			skippedEntries2 += 1;
		}

		if (skippedEntries1 > entries1/50 || skippedEntries2 > entries2/50) {
			if (skippedEntries1 > 10 && skippedEntries2 > 10) {
				cout << "Skipping too many events!" << endl;
				// Save graph
				gDdtime->Draw("ALP");
				c1->Print(graphPath);
				return 2;
			}
		}
	}

	// Check for leftover entries
	entry1 -= 1;
	entry2 -= 1;
	if (entry1 != entries1 - 1) {
		cout << "Leftover events " << entry1 << "~" << entries1 - 1 << "/" << entries1 - 1 << " in tree 1" << endl;
		skippedEntries1 += entries1 - 1 - entry1;
	} else if (entry2 != entries2 - 1) {
		cout << "Leftover events " << entry2 << "~" << entries2 - 1 << "/" << entries2 - 1 << " in tree 2" << endl;
		skippedEntries2 += entries2 - 1 - entry2;
	} else {
		cout << "No leftovers" << endl;
	}
	cout << "Total skipped events in tree 1: " << skippedEntries1 << "/" << entries1 - 1 << endl;
	cout << "Total skipped events in tree 2: " << skippedEntries2 << "/" << entries2 - 1 << endl;

	// Save output file
	outputTree->Write();
	outputFile->Close();
	inputFile1->Close();
	inputFile2->Close();
	cout << "Saved to " << outputPath << endl;

	// Save graph
	gDdtime->Draw("ALP");
	c1->Update();
	c1->Print(graphPath);

	return 0;
}
