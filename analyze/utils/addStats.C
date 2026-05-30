#ifndef GETSTATS_C
#define GETSTATS_C

#include <TH1.h>
#include <TStyle.h>
#include <TPaveStats.h>

void addStats(TH1* hist, const vector<TString>& values) {
	hist->SetStats(false);

    TPaveStats* stat = new TPaveStats(
        0.65,
        0.92 - (values.size() + 1) * 0.03,
        0.90,
        0.92,
        "NDC"
    );

    stat->SetTextFont(42);
    stat->SetTextSize(0.025);
	stat->AddText(hist->GetName());

    for (const TString& v : values) {
        stat->AddText(v);
	}

    hist->GetListOfFunctions()->Add(stat);
}

#endif
