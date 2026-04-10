#include <TTree.h>
#include <TString.h>
#include <TChain.h>
#include <vector>

using namespace std;

struct DataBase {
	TChain* tree = nullptr;
	Long64_t entries;

	DataBase() {};

	DataBase(const vector<TString>& paths, const char* treeName) {
		tree = new TChain(treeName);
		for (const TString& path : paths) {
			tree->Add(path);
		}
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
		const char* layers[3] = {"X","Y","U"};
		for (size_t i = 0; i < 3; i++) {
			tree->SetBranchAddress(Form("time%s", layers[i]), &timeV[i]);
			tree->SetBranchAddress(Form("energy%s", layers[i]), &energyV[i]);
			tree->SetBranchAddress(Form("tot%s", layers[i]), &totV[i]);
			tree->SetBranchAddress(Form("channelID%s", layers[i]), &channelIdV[i]);
			tree->SetBranchAddress(Form("xi%s", layers[i]), &xiV[i]);
		}
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

