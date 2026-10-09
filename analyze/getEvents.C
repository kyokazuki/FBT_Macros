#include <iostream>
#include <stdlib.h>

#include "utils/loadData.C"
#include "utils/printProgress.C"

void getEventsStart(
	const DataFBT2& inData
) {
	for (Int_t layer = 0; layer < 3; layer++) {
		inData.tree->SetBranchStatus(Form("tot%c", LAYER_NAMES[layer]), 1);
		inData.tree->SetBranchStatus(Form("xi%c", LAYER_NAMES[layer]), 1);
	}
}

Int_t getEventsLoop(
	const DataFBT2& inData
) {
	Double_t minTot = 0.5;

	// coincident event
	for (Int_t layer = 0; layer < 3; layer++) {
		if (inData.totV[layer]->size() == 0) {
			return 0;
		}
		if ((*inData.totV[layer])[0] < minTot) {
			return 0;
		}
	}

	// small misalignment
	Double_t misalignment = abs(
		(Double_t)((*inData.xiV[0])[0] + (*inData.xiV[1])[0])
		- ((Double_t)((*inData.xiV[2])[0]) - MISALIGNMENT_OFFSET) / MISALIGNMENT_SLOPE
	);
	if (misalignment > 5) {
		return 0;
	}

	Int_t mainLayer = 0;

	// single-hit on main layer
	// if (inData.totV[mainLayer]->size() >= 2 && (*inData.totV[mainLayer])[1] > minTot) {
	// 	return 0;
	// }

	// multi-hit on main layer
	if (!(inData.totV[mainLayer]->size() >= 2 && (*inData.totV[mainLayer])[1] > minTot)) {
		return 0;
	}

	// single-hit on other layers
	if (inData.totV[(mainLayer+1)%3]->size() >= 2 && (*inData.totV[(mainLayer+1)%3])[1] > minTot) {
		return 0;
	}
	if (inData.totV[(mainLayer+2)%3]->size() >= 2 && (*inData.totV[(mainLayer+2)%3])[1] > minTot) {
		return 0;
	}

	return 1;
}

void getEventsEnd(
	const DataBase& inData
) {
}

Long64_t getEvents(
	const vector<TString>& inPaths
) {
	DataFBT2 inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	getEventsStart(inData);

	// loop through all events
	Long64_t totalEvents = 0;
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		totalEvents += getEventsLoop(inData);
	}

	getEventsEnd(inData);

	return totalEvents;
}

