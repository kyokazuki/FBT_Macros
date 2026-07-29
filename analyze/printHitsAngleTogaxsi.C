#include "cmath"

#include "printHitsAngle.C"

void printHitsAngleTogaxsiStart(
	const DataFBTTOGAXSI& inData,
	const TString& outName
) {
	// rc_vtx //////////////////////////////////////////////
	// inData.tree->SetBranchStatus("rc_vtx", 1);
	// inData.tree->SetBranchStatus("rc_is_in_tgt", 1);
	// br_vtx //////////////////////////////////////////////
	inData.tree->SetBranchStatus("br_vtx", 1);
	inData.tree->SetBranchStatus("br_is_in_tgt", 1);
	// bc_vtx //////////////////////////////////////////////
	// inData.tree->SetBranchStatus("bc_vtx", 1);
	// inData.tree->SetBranchStatus("bc_is_in_tgt", 1);

	printHitsAngleStart(inData, outName);
}

int printHitsAngleTogaxsiLoop(
	const DataFBTTOGAXSI& inData,
	UInt_t hits,
	const TString& outName,
	Bool_t saveGraph
) {
	// select recoil vertex
	UInt_t vertexes = 0;
	UInt_t index1 = 0;
	// rc_vtx //////////////////////////////////////////////
	// UInt_t index2 = 0;
	// for (UInt_t recoil = 0; recoil < 4; recoil++) {
	// 	for (UInt_t cluster = 0; cluster < 2; cluster++) {
	// 		if (isnan(inData.rc_vtx[recoil][cluster][0])) {
	// 			continue;
	// 		}
	// 		if (inData.rc_is_in_tgt[recoil][cluster] == 0) {
	// 			continue;
	// 		}
	// 		index1 = recoil;
	// 		index2 = cluster;
	// 		vertexes++;
	// 	}
	// }
	// br_vtx //////////////////////////////////////////////
	for (UInt_t recoil = 0; recoil < 4; recoil++) {
		if (isnan(inData.br_vtx[recoil][0])) {
			continue;
		}
		if (inData.br_is_in_tgt[recoil] == 0) {
			continue;
		}
		index1 = recoil;
		vertexes++;
	}
	// bc_vtx //////////////////////////////////////////////
	// for (UInt_t cluster = 0; cluster < 2; recoil++) {
	// 	if (isnan(inData.bc_vtx[cluster][0])) {
	// 		continue;
	// 	}
	// 	if (inData.bc_is_in_tgt[cluster] == 0) {
	// 		continue;
	// 	}
	// 	index1 = cluster;
	// 	vertexes++;
	// }
	////////////////////////////////////////////////////////

	if (vertexes == 1) {
		// rc_vtx //////////////////////////////////////////
		// Double_t reactionCoord[3] = {
		// 	inData.rc_vtx[index1][index2][0], 
		// 	inData.rc_vtx[index1][index2][1], 
		// 	inData.rc_vtx[index1][index2][2]
		// };
		// br_vtx //////////////////////////////////////////
		Double_t reactionCoord[3] = {
			inData.br_vtx[index1][0], 
			inData.br_vtx[index1][1], 
			inData.br_vtx[index1][2]
		};
		// bc_vtx //////////////////////////////////////////
		// Double_t reactionCoord[3] = {
		// 	inData.bc_vtx[index1][0], 
		// 	inData.bc_vtx[index1][1], 
		// 	inData.bc_vtx[index1][2]
		// };
		////////////////////////////////////////////////////
		return printHitsAngleLoop(inData, reactionCoord, hits, outName, saveGraph);
	} else {
		return 0;
	}
}

void printHitsAngleTogaxsiEnd(
	const DataBase& inData,
	const TString& outName
) {
	printHitsAngleEnd(inData, outName);
}

void printHitsAngleTogaxsi(
	const vector<TString>& inPaths,
	UInt_t hits = 1,
	Long64_t loopEvents = 100
) {
	// load data
	DataFBTTOGAXSI inData(inPaths, "events");
	inData.tree->SetBranchStatus("*", 0);

	TString outName = Form("%s_hitsAngleTogaxsi_%dhits.pdf", inData.runNum.Data(), hits);
	printHitsAngleTogaxsiStart(inData, outName);

	// only draw loopEvents of events
	Long64_t drawnEvents = 0;
	if (loopEvents == 0) {
		loopEvents = inData.entries;
	}

	// event loop
	for (Long64_t entry = 0; entry < inData.entries; entry++) {
		printProgress(entry, inData.entries);
		inData.tree->GetEntry(entry);

		if (drawnEvents < loopEvents) {
			drawnEvents += printHitsAngleTogaxsiLoop(inData, hits, outName, 1);
		} else {
			printHitsAngleTogaxsiLoop(inData, hits, outName, 0);
		}
	}

	printHitsAngleTogaxsiEnd(inData, outName);
}

