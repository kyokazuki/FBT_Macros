#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <iostream>

#include "plot_time_diff_gate.C"

#include "plot_xy_u.C"
#include "get_effcy_grouped.C"

void analyzeSingles(const char *fileName) {
	TFile *inputFile = TFile::Open(fileName);
	TTree *tree = (TTree*)inputFile->Get("data");

	Long64_t fiberEntries = tree->GetEntries("!((xi == 0 && energy == 5) || (xi == 0 && energy == -5))");
	Long64_t gateEntries = tree->GetEntries("xi == 0 && energy == 5");

	gStyle->SetOptStat(11111111);
	TCanvas *c2 = new TCanvas("c2","c2"); 
	c2->Divide(3,2);

	c2->cd(1);
	tree->Draw("time", "xi == 0 && energy == 5");

	c2->cd(2);
	time_diff(fileName);
	gPad->SetLogz();
	hdt_tot->Draw();

	c2->cd(3);

	c2->cd(4);
	gPad->SetLogz();
	tree->Draw("tot:xi>>htotX(320, 0.5, 320.5, 100, 0, 300000)", "yi==0", "colz");

	c2->cd(5);
	gPad->SetLogz();
	tree->Draw("tot:xi>>htotY(320, 0.5, 320.5, 100, 0, 300000)", "yi==1", "colz");

	c2->cd(6);
	gPad->SetLogz();
	tree->Draw("tot:xi>>htotU(320, 0.5, 320.5, 100, 0, 300000)", "yi==2", "colz");

	cout << "Total fiber entries: " << fiberEntries << endl;
	cout << "Total gate entries: " << gateEntries << endl;

}

void analyzeFriended(const char *fileName) {
	TFile *inputFile = TFile::Open(fileName);
	TTree *tree = (TTree*)inputFile->Get("events");

	Long64_t eventEntries = tree->GetEntries();

	vector <Float_t> TOT_RANGE = {30000, 300000};
	Float_t effcyX = getEffcyGrouped(fileName, 30, 0, TOT_RANGE);
	Float_t effcyY = getEffcyGrouped(fileName, 30, 1, TOT_RANGE);
	Float_t effcyU = getEffcyGrouped(fileName, 30, 2, TOT_RANGE);
	cout << "Efficiency of (X, Y, U): (" << effcyX << ", " << effcyY << ", " << effcyU << ")" << endl;

	gStyle->SetOptStat(11111111);
	TCanvas *c2 = new TCanvas("c2","c2"); 
	c2->Divide(2, 2);

	c2->cd(1);
	plot_xy_u(fileName);
	hxy_u->Draw();

	c2->cd(2);


	c2->cd(3);
	gPad->SetLogz();
	tree->Draw("xiX:(l1t-l2t)>>hxiX-lt(100, -10, 90, 320, 0.5, 320.5)", "totX > 30000", "colz");

	c2->cd(4);
	gPad->SetLogz();
	tree->Draw("xiX:(m1t-m2t)>>hxiX-mt(100, -50, 50, 320, 0.5, 320.5)", "totX > 30000", "colz");
}
