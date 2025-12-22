#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include <iostream>
#include <stdlib.h>

#include <unistd.h>

void checkEvents(const TString& inputPath1, const TString& inputPath2, Long64_t entries) {
	TFile* inputFile1 = TFile::Open(inputPath1);
	TTree* inputTree1 = (TTree*)inputFile1->Get("data");
	TFile* inputFile2 = TFile::Open(inputPath2);
	TTree* inputTree2 = (TTree*)inputFile2->Get("tree");

	Long64_t time;
	inputTree2->SetBranchAddress("time", &time);

	Long64_t bbtime;
	inputTree2->SetBranchAddress("bbtime", &bbtime);
	
	Long64_t entries1 = inputTree1->GetEntries("xi == 0 && energy == 5");
	Long64_t entries2 = inputTree2->GetEntries();
	cout << "Input File 1: " << entries1 << " entries" << endl;
	cout << "Input File 2: " << entries2 << " entries" << endl;
	if (entries1 == entries2) {
		cout << "Gate counts match!" << endl;
	} else {
		cout << "Gate counts don't match!" << endl;
		exit(1);
	}

	Long64_t lastValue1 = 0;
	cout << "File 1 (first " << entries << " events):" << endl;
	cout << "entry	time	diff" << endl;
		for (Long64_t entry = 0; entry < entries1; entry++) {
			inputTree1->GetEntry(entry);
			lastValue1 = time - lastValue1;
			if (entry == 0) {
				cout << entry << "	" << time << endl;
			} else {
				cout << entry << "	" << time << "	" << lastValue1 << endl;
			}
		}

	Long64_t lastValue2 = 0;
	cout << "File 2 (first " << entries << " events):" << endl;
	cout << "entry	time	diff" << endl;
		for (Long64_t entry = 0; entry < entries; entry++) {
			inputTree2->GetEntry(entry);
			lastValue2 = time - lastValue2;
			if (entry == 0) {
				cout << entry << "	" << time << endl;
			} else {
				cout << entry << "	" << time << "	" << lastValue2 << endl;
			}
		}

	lastValue1 = 0;
	cout << "File 1 (last " << entries << " events):" << endl;
	cout << "entry	time	diff" << endl;
		for (Long64_t entry = entries1 - 1; entry > entries1 - entries - 1; entry = entry - 1) {
			inputTree1->GetEntry(entry);
			lastValue1 = time - lastValue1;
			if (entry == 0) {
				cout << entry << "	" << time << endl;
			} else {
				cout << entry << "	" << time << "	" << lastValue1 << endl;
			}
		}

	lastValue2 = 0;
	cout << "File 2 (last " << entries << " events):" << endl;
	cout << "entry	time	diff" << endl;
		for (Long64_t entry = entries2 - 1; entry > entries2 - entries - 1; entry = entry - 1) {
			inputTree2->GetEntry(entry);
			lastValue2 = time - lastValue2;
			if (entry == 0) {
				cout << entry << "	" << time << endl;
			} else {
				cout << entry << "	" << time << "	" << lastValue2 << endl;
			}
		}

	inputFile1->Close();
	inputFile2->Close();
}
