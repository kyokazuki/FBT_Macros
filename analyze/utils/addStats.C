#ifndef GETSTATS_C
#define GETSTATS_C

#include <vector>
#include <sstream>

#include <TH1.h>
#include <TString.h>
#include <TStyle.h>
#include <TPaveStats.h>
#include <TList.h>

template <typename T>
TString getVecString(const std::vector<T>& vec) {
    ostringstream os;
    for (size_t i = 0; i < vec.size(); i++) {
        if (i) os << ", ";
        os << vec[i];
    }

    return TString(os.str());
}

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

    for (const TString& v : values) {
        stat->AddText(v);
	}

    hist->GetListOfFunctions()->Add(stat);
}

#endif
