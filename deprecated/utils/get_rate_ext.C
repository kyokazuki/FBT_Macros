#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>

Long64_t get_rate_ext(
	const char *fname, 
	Double_t ext_gate_width, 
	Int_t xi_min, Int_t xi_max, 
	Float_t tot_min, Float_t tot_max
) {

	Int_t xi_count = xi_max - xi_min + 1;
	Double_t live_time; 
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
			"xi==0 || (xi>=%d && xi<=%d && tot>=%f && tot<=%f)", 
			xi_min, xi_max, tot_min, tot_max
			), 
		"goff"
	);
	TH1F *hxi = (TH1F*) gDirectory->Get("hxi");

	live_time = hxi->GetBinContent(1)/2 * ext_gate_width;
	if (live_time == 0) {
		return 0;
	}

	for (Int_t i=1; i<=xi_count; i++) {
		rate_avg += hxi->GetBinContent(i+1);
	}
	rate_avg = floor(rate_avg / xi_count / live_time);

	inputFile->Close();

	return rate_avg;
}
