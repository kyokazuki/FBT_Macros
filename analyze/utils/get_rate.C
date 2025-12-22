#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

Long64_t get_rate(
	const char *fname, 
	Int_t time, 
	Int_t xi_min, Int_t xi_max, 
	Float_t tot_min, Float_t tot_max
) {

	Int_t xi_count = xi_max - xi_min + 1;
	Double_t rate_avg = 0;

	TFile *inputFile = TFile::Open(fname);
	TTree *tree = (TTree*)inputFile->Get("data");
	if (tree->GetEntries() == 0) {
		inputFile->Close();
		return 0;
	}
	tree->Draw(
		"xi>>hxi", 
		Form(
			"xi>=%d && xi<=%d && tot>=%f && tot<=%f", 
			xi_min, xi_max, tot_min, tot_max
			), 
		"goff"
	);
	TH1F *hxi = (TH1F*) gDirectory->Get("hxi");

	for (Int_t i=1; i<=xi_count; i++) {
		rate_avg += hxi->GetBinContent(i);
	}
	rate_avg = floor(rate_avg / xi_count / time);

	inputFile->Close();

	return rate_avg;
}
