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

#include "utils/parameters.C"
#include "utils/loadData.C"
#include "utils/createOutFile.C"
#include "utils/printProgress.C"

int processFriendDelta(const TString& inPath1, const TString& inPath2) {
	cout << "Friending " << inPath1 << " with " << inPath2 << endl;

	bool SAVE_GRAPH = 1;
	TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
	TGraph *gDdtime = nullptr;
	TString runNumber = TString(gSystem->BaseName(inPath1))(0,4);

	DataFBT2 inData1({inPath1}, "events");
	DataVME2 inData2({inPath2}, "tree");

	TFile* outFile = createOutFile(inPath1, "_friended.root");
	TTree* outTree = inData1.tree->CloneTree(0);
	outTree->Branch("l1t", &inData2.l1t);
	outTree->Branch("l2t", &inData2.l2t);
	outTree->Branch("m1t", &inData2.m1t);
	outTree->Branch("m2t", &inData2.m2t);
	outTree->Branch("s1t", &inData2.s1t);
	outTree->Branch("l1q", &inData2.l1q);
	outTree->Branch("l2q", &inData2.l2q);
	outTree->Branch("m1q", &inData2.m1q);
	outTree->Branch("m2q", &inData2.m2q);
	outTree->Branch("s1q", &inData2.s1q);
	outTree->Branch("rft", &inData2.rft);
	outTree->Branch("bbtime", &inData2.bbtime);
	outTree->Branch("scaler", &inData2.scaler, "scaler[32]/I");
	Long64_t dtime, dbbtime;
	outTree->Branch("dtime", &dtime);
	outTree->Branch("dbbtime", &dbbtime);

	// Get input trees' entry counts
	cout << "Input File 1: " << inData1.entries << " entries" << endl;
	cout << "Input File 2: " << inData2.entries << " entries" << endl;

	// Set appropriate time ratio and time resolution
	inData1.tree->GetEntry(0);
	Long64_t firstTime1 = (*inData1.timeGate)[0];
	inData1.tree->GetEntry(inData1.entries - 1);
	Long64_t lastTime1 = (*inData1.timeGate)[0];
	inData2.tree->GetEntry(0);
	Long64_t firstTime2 = inData2.bbtime;
	inData2.tree->GetEntry(inData2.entries - 1);
	Long64_t lastTime2 = inData2.bbtime;
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
	if (inData1.entries == inData2.entries) {
		startEntry1 = 0;
		startEntry2 = 0;
		cout << "Event 0/" << inData1.entries - 1 << " in tree 1 matched with event 0/" << inData2.entries - 1 << " in tree 2" << endl;
	} else if (inData1.entries < inData2.entries) {
		inData1.tree->GetEntry(0);
		startTime1 = (*inData1.timeGate)[0];
		inData1.tree->GetEntry(1);
		endTime1 = (*inData1.timeGate)[0];
		dTime1 = endTime1 - startTime1;

		for (entry2 = 0; entry2 <= inData2.entries - inData1.entries; entry2++) {
			inData2.tree->GetEntry(entry2);
			startTime2 = inData2.bbtime;
			inData2.tree->GetEntry(entry2 + 1);
			endTime2 = inData2.bbtime;
			dTime2 = endTime2 - startTime2;

			if (dTime1 >= dTime2 * timeRatio - timeRes && dTime1 <= dTime2 * timeRatio + timeRes) {
				cout << "Event 0/" << inData1.entries - 1 << " in tree 1 matched with event " << entry2 << "/" << inData2.entries - 1 << " in tree 2" << endl;
				startEntry1 = 0;
				startEntry2 = entry2;
				break;
			}
		}
	} else if (inData1.entries > inData2.entries) {
		inData2.tree->GetEntry(0);
		startTime2 = inData2.bbtime;
		inData2.tree->GetEntry(1);
		endTime2 = inData2.bbtime;
		dTime2 = endTime2 - startTime2;

		for (entry1 = 0; entry1 <= inData1.entries - inData2.entries; entry1++) {
			inData1.tree->GetEntry(entry1);
			startTime1 = (*inData1.timeGate)[0];
			inData1.tree->GetEntry(entry1 + 1);
			endTime1 = (*inData1.timeGate)[0];
			dTime1 = endTime1 - startTime1;

			if (dTime1 >= dTime2 * timeRatio - timeRes && dTime1 <= dTime2 * timeRatio + timeRes) {
				cout << "Event " << entry1 << "/" << inData1.entries - 1 << " in tree 1 matched with event 0/" << inData2.entries - 1 << " in tree 2" << endl;
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
	TString graphPath = Form("%s_friendDelta.pdf", runNumber.Data());
	if (SAVE_GRAPH) {
		c1->SetGridx(true);
		c1->SetGridy(true);
		gDdtime = new TGraph();
		gDdtime->SetTitle("Ddtime;babi time[~ns];d(FBT time)-d(babi time)*timeRatio[ps]");
		gDdtime->GetXaxis()->SetLimits(firstTime2, lastTime2);
		// gDdtime->SetMaximum(timeRes);
		// gDdtime->SetMinimum(-1*timeRes);
	}

	// Go through all entries and match events
	inData1.tree->GetEntry(startEntry1);
	inData2.tree->GetEntry(startEntry2);
	dtime = 0;
	dbbtime = 0;
	outTree->Fill();
	startTime1 = (*inData1.timeGate)[0];
	startTime2 = inData2.bbtime;
	Long64_t skippedEntries1 = startEntry1;
	Long64_t skippedEntries2 = startEntry2;
	bool goNext1 = 1, goNext2 = 1;
	Long64_t graphPoint = 0;
	for (entry1 = startEntry1 + 1, entry2 = startEntry2 + 1; entry1 < inData1.entries && entry2 < inData2.entries; entry1 += goNext1, entry2 += goNext2) {
		printProgress(entry2, inData2.entries);

		inData1.tree->GetEntry(entry1);
		inData2.tree->GetEntry(entry2);
		endTime1 = (*inData1.timeGate)[0];
		endTime2 = inData2.bbtime;
		dTime1 = endTime1 - startTime1;
		dTime2 = endTime2 - startTime2;

		if (SAVE_GRAPH && entry1 % 100 == 0) {
			gDdtime->SetPoint(graphPoint++, endTime2, dTime1 - dTime2 * timeRatio);
		}

		if (dTime1 >= dTime2 * timeRatio - timeRes && dTime1 <= dTime2 * timeRatio + timeRes) {
			dtime = dTime1;
			dbbtime = dTime2;
			outTree->Fill();
			startTime1 = (*inData1.timeGate)[0];
			startTime2 = inData2.bbtime;
			goNext1 = 1;
			goNext2 = 1;
		} else if (dTime1 < dTime2 * timeRatio - timeRes) {
			cout << "Skipping event " << entry1 << "/" << inData1.entries - 1 << " in tree 1 (" << dTime1 << ":" << dTime2 << ")" << endl;
			goNext1 = 1;
			goNext2 = 0;
			skippedEntries1 += 1;
		} else if (dTime1 > dTime2 * timeRatio + timeRes) {
			cout << "Skipping event " << entry2 << "/" << inData2.entries - 1 << " in tree 2 (" << dTime1 << ":" << dTime2 << ")" << endl;
			goNext1 = 0;
			goNext2 = 1;
			skippedEntries2 += 1;
		}

		if (skippedEntries1 > inData1.entries/50 || skippedEntries2 > inData2.entries/50) {
			if (skippedEntries1 > 10 && skippedEntries2 > 10) {
				cout << "Skipping too many events!" << endl;
				outTree->Write();
				outFile->Close();
				// gSystem->Unlink(outPath);

				if (SAVE_GRAPH) {
					gDdtime->Draw("ALP");
					c1->Print(graphPath);
				}
				return 2;
			}
		}
	}

	// Check for leftover entries
	entry1 -= 1;
	entry2 -= 1;
	if (entry1 != inData1.entries - 1) {
		cout << "Leftover events " << entry1 << "~" << inData1.entries - 1 << "/" << inData1.entries - 1 << " in tree 1" << endl;
		skippedEntries1 += inData1.entries - 1 - entry1;
	} else if (entry2 != inData2.entries - 1) {
		cout << "Leftover events " << entry2 << "~" << inData2.entries - 1 << "/" << inData2.entries - 1 << " in tree 2" << endl;
		skippedEntries2 += inData2.entries - 1 - entry2;
	} else {
		cout << "No leftovers" << endl;
	}
	cout << "Total skipped events in tree 1: " << skippedEntries1 << "/" << inData1.entries - 1 << endl;
	cout << "Total skipped events in tree 2: " << skippedEntries2 << "/" << inData2.entries - 1 << endl;

	// Save output file
	outTree->Write();
	outFile->Close();

	// Save graph
	if (SAVE_GRAPH) {
		gDdtime->Draw("ALP");
		c1->Update();
		c1->Print(graphPath);
		delete c1;
		delete gDdtime;
	}

	return 0;
}
