#include <TFile.h>
#include <TString.h>

TFile* createOutFile(const TString& inPath, const TString& suffix) {
    TString outPath = inPath;
    outPath.ReplaceAll(".root", suffix);

    TFile* outFile = new TFile(outPath, "RECREATE");
	cout << endl << "Created " << outPath << endl;

    outFile->cd();

    return outFile;
}

