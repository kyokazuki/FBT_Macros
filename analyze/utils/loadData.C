#ifndef LOADDATA_C
#define LOADDATA_C 

#include <iostream>
#include <vector>
#include <cmath>

#include <TTree.h>
#include <TString.h>
#include <TChain.h>

#include "constants.C"
#include "inRange.C"
#include "getRunNumber.C"

using namespace std;

struct DataBase {
	TChain* tree = nullptr;
	Long64_t entries;
	TString runNum;

	DataBase() {};

	DataBase(const vector<TString>& paths, const char* treeName) {
		tree = new TChain(treeName);
		for (const TString& path : paths) {
			tree->Add(path);
		}
		runNum = getRunNumberMult(paths);

		entries = tree->GetEntries();
	}

	virtual ~DataBase() {
		delete tree;
	}
};

struct DataFBT1 : virtual public DataBase {
	Long64_t time; 
	Float_t energy;
	Float_t tot;
	UInt_t channelId; 
	Int_t xi, yi;

	DataFBT1(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("time", &time);
		tree->SetBranchAddress("energy", &energy);
		tree->SetBranchAddress("tot", &tot);
		tree->SetBranchAddress("channelID", &channelId);
		tree->SetBranchAddress("xi", &xi);
		tree->SetBranchAddress("yi", &yi);
    }
};

struct DataFBT2 : virtual public DataBase {
	vector<vector<Long64_t>*> 	timeV{3, nullptr};
	vector<vector<Float_t>*> 	energyV{3, nullptr};
	vector<vector<Float_t>*> 	totV{3, nullptr};
	vector<vector<UInt_t>*> 	channelIdV{3, nullptr};
	vector<vector<Int_t>*> 		xiV{3, nullptr};
	vector<Long64_t>* 			timeGate = nullptr;

	DataFBT2(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("timeGate", &timeGate);
		for (Int_t layer = 0; layer < 3; layer++) {
			tree->SetBranchAddress(Form("time%c", LAYER_NAMES[layer]), &timeV[layer]);
			tree->SetBranchAddress(Form("energy%c", LAYER_NAMES[layer]), &energyV[layer]);
			tree->SetBranchAddress(Form("tot%c", LAYER_NAMES[layer]), &totV[layer]);
			tree->SetBranchAddress(Form("channelID%c", LAYER_NAMES[layer]), &channelIdV[layer]);
			tree->SetBranchAddress(Form("xi%c", LAYER_NAMES[layer]), &xiV[layer]);
		}
    }

	void clear() {
		timeGate->clear();
		for (Int_t layer = 0; layer < 3; layer++) {
			timeV[layer]->clear();
			energyV[layer]->clear();
			totV[layer]->clear();
			channelIdV[layer]->clear();
			xiV[layer]->clear();
		}
	}

	Int_t getMaxTotLayer(
		const Float_t (&totRange)[2]		= MAX_TOT_RANGE, 
		const Long64_t (&timingRange)[2]	= MAX_TIMING_RANGE
	) const {
		Float_t maxTot = -1;
		Int_t maxTotLayer = -1;

		for (Int_t layer = 0; layer < 3; layer++) {
			// check if all layers are hit
			if (totV[layer]->empty()) {
				return -1;
			}

			// check if is in right timing
			if (!inRange((*timeV[layer])[0] - (*timeGate)[0], timingRange)) {
				return -1;
			}

			// update max tot value
			if ((*totV[layer])[0] > maxTot && inRange((*totV[layer])[0], totRange)) {
				maxTotLayer = layer;
				maxTot = (*totV[layer])[0];
			}
		}
		return maxTotLayer;
	}
};

struct DataVME1 : virtual public DataBase {
	Double_t ult, urt, dlt, drt, ulq, urq, dlq, drq;
	Long64_t scaler;

	DataVME1(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("ult", &ult);
		tree->SetBranchAddress("urt", &urt);
		tree->SetBranchAddress("dlt", &dlt);
		tree->SetBranchAddress("drt", &drt);
		tree->SetBranchAddress("ulq", &ulq);
		tree->SetBranchAddress("urq", &urq);
		tree->SetBranchAddress("dlq", &dlq);
		tree->SetBranchAddress("drq", &drq);
		tree->SetBranchAddress("scaler32", &scaler);
    }
};

struct DataFBT3 : public DataFBT2, public DataVME1 {
	DataFBT3(const vector<TString>& paths, const TString& treeName)
		: DataBase(paths, treeName), DataFBT2(paths, treeName), DataVME1(paths, treeName) {}
};

struct DataVME2 : virtual public DataBase {
    Double_t l1t, l2t, m1t, m2t, s1t;
    Double_t l1q, l2q, m1q, m2q, s1q;
    Double_t rft;
    Long64_t bbtime;
    Int_t scaler[32];

    DataVME2(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
        tree->SetBranchAddress("l1t", &l1t);
        tree->SetBranchAddress("l2t", &l2t);
        tree->SetBranchAddress("m1t", &m1t);
        tree->SetBranchAddress("m2t", &m2t);
        tree->SetBranchAddress("s1t", &s1t);

        tree->SetBranchAddress("l1q", &l1q);
        tree->SetBranchAddress("l2q", &l2q);
        tree->SetBranchAddress("m1q", &m1q);
        tree->SetBranchAddress("m2q", &m2q);
        tree->SetBranchAddress("s1q", &s1q);

        tree->SetBranchAddress("rft", &rft);
        tree->SetBranchAddress("bbtime", &bbtime);
        tree->SetBranchAddress("scaler", scaler);
    }
};

struct DataFBT4 : public DataFBT2, public DataVME2 {
	DataFBT4(const vector<TString>& paths, const TString& treeName)
		: DataBase(paths, treeName), DataFBT2(paths, treeName), DataVME2(paths, treeName) {}
};

struct DataFBT5 : public DataFBT4 {
	Float_t rate;

	DataFBT5(const vector<TString>& paths, const TString& treeName)
		: DataBase(paths, treeName), DataFBT4(paths, treeName)
	{
		tree->SetBranchAddress("rate", &rate);
	}
};

struct DataTS : virtual public DataBase {
	ULong64_t smts;

	DataTS(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("SMTS", &smts);
    }
};

struct DataFDC1 : virtual public DataBase {
	ULong64_t ts;
	Double_t wirez[1000];
	Double_t wirepos[1000];

	DataFDC1(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("ts", &ts);
		tree->SetBranchAddress("fdc1ht.wirez", wirez);
		tree->SetBranchAddress("fdc1ht.wirepos", wirepos);
    }
};

struct DataCoin : virtual public DataBase {
	Bool_t coin[16];

	DataCoin(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("coin", coin);
    }
};

struct DataFBTCoin : public DataFBT2, public DataCoin {
	DataFBTCoin(const vector<TString>& paths, const TString& treeName)
		: DataBase(paths, treeName), DataFBT2(paths, treeName), DataCoin(paths, treeName) {}
};

struct DataHodo : virtual public DataBase {
	Bool_t coin[16];
	Int_t fID[40];
	Double_t fQCal[40];
	Double_t fTUCal[40];
	Double_t fTDCal[40];

	DataHodo(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("coin", coin);
		tree->SetBranchAddress("fID", fID);
		tree->SetBranchAddress("fQCal", fQCal);
		tree->SetBranchAddress("fTUCal", fTUCal);
		tree->SetBranchAddress("fTDCal", fTDCal);
    }

	Int_t getIndex(Int_t id) const {
		// hodo id madness
		Int_t index;
		if (id >= 0 && id <= 23) {
			return id;
		} else if (id >= 25 && id <= 40) {
			return id - 1;
		} else {
			return -1;
		}
	}

	Int_t getCoinOnes(const vector<Int_t>& coins) const {
		Int_t coinOnes = 0;
		for (UInt_t i = 0; i < coins.size(); i++) {
			if (coin[coins[i]] == 1) {
				coinOnes++;
			}
		}
		return coinOnes;
	}

	Int_t getMaxQId(const Int_t (&idRange)[2], const Double_t (&tRange)[2], const Double_t (&qRange)[2]) const {
		Float_t maxQ = -1;
		Int_t maxQId = -1;
		for (Int_t id = idRange[0]; id <= idRange[1]; id++) {
			Int_t index = getIndex(id);
			if (index == -1) {
				continue;
			}

			if (isnan(fQCal[index])) {
				continue;
			}

			// check for the first beam bunch
			if (!inRange((fTUCal[index] + fTDCal[index]) / 2, tRange)) {
				continue;
			}

			if (fQCal[index] > maxQ && inRange(fQCal[index], qRange)) {
				maxQ = fQCal[index];
				maxQId = id;
			}
		}
		return maxQId;
	}
};

struct DataFBTHodo : public DataFBT2, public DataHodo {
	DataFBTHodo(const vector<TString>& paths, const TString& treeName)
		: DataBase(paths, treeName), DataFBT2(paths, treeName), DataHodo(paths, treeName) {}
};

struct DataTOGAXSI : virtual public DataBase {
	Double_t rc_vtx[4][2][3];
	Double_t br_vtx[4][3];
	Double_t bc_vtx[2][3];
	Bool_t rc_is_in_tgt[4][2];
	Bool_t br_is_in_tgt[4];
	Bool_t bc_is_in_tgt[4];

	DataTOGAXSI(const vector<TString>& paths, const TString& treeName) : DataBase(paths, treeName) {
		tree->SetBranchAddress("rc_vtx", rc_vtx);
		tree->SetBranchAddress("br_vtx", br_vtx);
		tree->SetBranchAddress("bc_vtx", bc_vtx);
		tree->SetBranchAddress("rc_is_in_tgt", rc_is_in_tgt);
		tree->SetBranchAddress("br_is_in_tgt", br_is_in_tgt);
		tree->SetBranchAddress("bc_is_in_tgt", bc_is_in_tgt);
    }
};

struct DataFBTTOGAXSI : public DataFBT2, public DataTOGAXSI {
	DataFBTTOGAXSI(const vector<TString>& paths, const TString& treeName)
		: DataBase(paths, treeName), DataFBT2(paths, treeName), DataTOGAXSI(paths, treeName) {}
};

#endif
