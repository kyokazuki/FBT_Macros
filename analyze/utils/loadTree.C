#include <TFile.h>
#include <TTree.h>

struct Data {
    TFile* file = nullptr;
    TTree* tree = nullptr;

	Long64_t time; 
	Float_t energy;
	Float_t tot;
	UInt_t channelId; 
	Int_t xi;
	Int_t yi;

    void loadTree(const char* filename) {
        file = TFile::Open(filename);
        if (!file || file->IsZombie()) {
            printf("Error opening file\n");
			exit(1);
        }

        tree = (TTree*) file->Get("data");
        if (!tree) {
            printf("Tree not found\n");
			exit(1);
        }
    }

    void loadBranches() {
	tree->SetBranchAddress("time", &time);
	tree->SetBranchAddress("energy", &energy);
	tree->SetBranchAddress("tot", &tot);
	tree->SetBranchAddress("channelID", &channelId);
	tree->SetBranchAddress("xi", &xi);
	tree->SetBranchAddress("yi", &yi);
    }
};

struct Data {
    TFile* file = nullptr;
    TTree* tree = nullptr;

    Double_t l1t, l2t, m1t, m2t, s1t;
    Double_t l1q, l2q, m1q, m2q, s1q;
    Double_t rft;
    Long64_t bbtime;
    Int_t scaler[32];

    void loadTree(const char* filename) {
        file = TFile::Open(filename);
        if (!file || file->IsZombie()) {
            printf("Error opening file\n");
			exit(1);
        }

        tree = (TTree*) file->Get("mtree");
        if (!tree) {
            printf("Tree not found\n");
			exit(1);
        }
    }

    void setupBranches() {
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

