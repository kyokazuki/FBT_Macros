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

#include "utils/loadData.C"
#include "utils/createOutFile.C"
#include "utils/printProgress.C"

struct Timer {
	Long64_t start;
	Long64_t end;
	Long64_t diff;

	void getDiff() {
		diff = end - start;
	}
};

void processPad(const TString& inPath1, const TString& inPath2) {
	cout << "Padding " << inPath1 << " with " << inPath2 << endl;

	// load input trees
	DataFBT2 inData1({inPath1}, "events");
	// DataFDC1 inData2({inPath2}, "tree"); // ts
	DataTS inData2({inPath2}, "t_TS"); // smts

	// check input trees' entry counts
	cout << "Tree1: " << inData1.entries << " entries" << endl;
	cout << "Tree2: " << inData2.entries << " entries" << endl;
	Long64_t entriesDiff = inData2.entries - inData1.entries;
	cout << "Entries diff: " << entriesDiff << endl;
	if (entriesDiff < 0) {
		if (entriesDiff == -1) {
			cout << "Tree1 has 1 more entry than tree2, ignoring last entry in Tree1" << endl;
			inData1.entries = inData1.entries - 1;
		} else {
			cout << "Tree1 has more entries than tree2!" << endl;
			return;
		}
	}

	// create output file
	TFile* outFile = createOutFile(inPath1, "_padded.root");
	TTree* outTree = inData1.tree->CloneTree(0);

	// get appropriate time ratio and time resolution
	Timer time1, time2;
	inData1.tree->GetEntry(0);
	time1.start = (*inData1.timeGate)[0];
	inData1.tree->GetEntry(inData1.entries - 1);
	time1.end = (*inData1.timeGate)[0];
	inData2.tree->GetEntry(0);
	time2.start = inData2.smts;
	inData2.tree->GetEntry(inData2.entries - 1);
	time2.end = inData2.smts;
	time1.getDiff();
	time2.getDiff();
	Double_t timeRatio = (Double_t)time1.diff / (Double_t)time2.diff;
	cout << setprecision(10) << "Time ratio: (" << time1.end << "-" << time1.start << ")/(" << time2.end << "-" << time2.start << ") = " << timeRatio << endl;
	Long64_t timeRes = 50000000;
	cout << "Time resolution: " << timeRes << endl;

	// look for the first matching event
	Long64_t entry1 = 0, entry2 = 0;
	Long64_t firstMatch = -1;
	inData1.tree->GetEntry(entry1);
	time1.start = (*inData1.timeGate)[0];
	inData1.tree->GetEntry(entry1 + 1);
	time1.end = (*inData1.timeGate)[0];
	time1.getDiff();
	for (entry2 = 0; entry2 <= inData2.entries - inData1.entries; entry2++) {
		inData2.tree->GetEntry(entry2);
		time2.start = inData2.smts;
		inData2.tree->GetEntry(entry2 + 1);
		time2.end = inData2.smts;
		time2.getDiff();
		if (time1.diff >= time2.diff * timeRatio - timeRes && time1.diff <= time2.diff * timeRatio + timeRes) {
			cout << "Matched first event: tree1[" << entry1 << "/" << inData1.entries - 1 << "] <-> tree2[" << entry2 << "/" << inData2.entries - 1 << "]" << endl;
			firstMatch = entry2;
			break;
		}
	}
	if (firstMatch == -1) {
		cout << "Unable to match first event!" << endl;
		return;
	}

	// fill the first events
	if (firstMatch > 0) {
		inData1.clear();
		for (Long64_t i = 0; i < firstMatch; i++) {
			outTree->Fill();
			cout << "Padded event: tree1[empty] <-> tree2[" << i << "/" << inData2.entries << "]" << endl;
		}
	}
	inData1.tree->GetEntry(entry1);
	outTree->Fill();
	inData1.tree->GetEntry(entry1 + 1);
	outTree->Fill();

	// go through all entries and match events
	time1.start = (*inData1.timeGate)[0];
	inData2.tree->GetEntry(firstMatch + 1);
	time2.start = inData2.smts;
	bool goNext1 = 1, goNext2 = 1;
	Long64_t pad = 0, padTotal = 0;
	for (entry1 = entry1 + 2, entry2 = firstMatch + 2; entry1 < inData1.entries && entry2 < inData2.entries; entry1 += goNext1, entry2 += goNext2) {
		printProgress(entry2, inData2.entries);

		inData1.tree->GetEntry(entry1);
		time1.end = (*inData1.timeGate)[0];
		time1.getDiff();
		inData2.tree->GetEntry(entry2);
		time2.end = inData2.smts;
		time2.getDiff();

		if (time1.diff >= time2.diff * timeRatio - timeRes * (pad + 1) && time1.diff <= time2.diff * timeRatio + timeRes * (pad + 1)) {
			outTree->Fill();
			time1.start = (*inData1.timeGate)[0];
			time2.start = inData2.smts;
			pad = 0;
			goNext1 = 1;
			goNext2 = 1;
		} else if (time1.diff > time2.diff * timeRatio + timeRes * (pad + 1)) {
			cout << "Padded event: tree1[empty] <-> tree2[" << entry2 << "/" << inData2.entries << "]" << endl;
			inData1.clear();
			outTree->Fill();
			pad += 1;
			padTotal += 1;
			goNext1 = 0;
			goNext2 = 1;
		} else if (time1.diff < time2.diff * timeRatio - timeRes * (pad + 1)) {
			cout << "Missing event: tree1[" << entry1 << "/" << inData1.entries << "] <-> tree2[empty]?" << endl;
			outTree->Write();
			outFile->Close();
			return;
		}

		if (padTotal > inData2.entries - inData1.entries) {
			cout << "Padding too many events!" << endl;
			outTree->Write();
			outFile->Close();
			return;
		}
	}

	// summery
	cout << "Padded " << padTotal << " events in tree1" << endl;

	// save output file
	outTree->Write();
	outFile->Close();

	return;
}
