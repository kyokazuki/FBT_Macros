#ifndef GETRUNNUMBER_C
#define GETRUNNUMBER_C

#include <cctype>
#include <vector>
#include <algorithm>

#include <TString.h>
#include <TSystem.h>

TString getRunNumber(const TString& path) {
	TString filename = gSystem->BaseName(path);
    for (Int_t i = 0; i <= filename.Length() - 4; ++i) {
        if (isdigit(filename[i]) &&
            isdigit(filename[i+1]) &&
            isdigit(filename[i+2]) &&
            isdigit(filename[i+3])) {

            return filename(i, 4);  // return 4-character substring
        }
    }

    return ""; // no 4-digit sequence found
}

TString getRunNumberMult(const std::vector<TString>& paths) {
    vector<Int_t> runs;

    // Extract run numbers
    for (const TString& path : paths) {
        TString runStr = getRunNumber(path);
		runs.push_back(runStr.Atoi());
    }

    if (runs.empty())
        return "";

    // Sort and remove duplicates
    std::sort(runs.begin(), runs.end());
    // runs.erase(std::unique(runs.begin(), runs.end()), runs.end());

    TString result;

    Int_t start = runs[0];
    Int_t prev  = runs[0];

    auto appendRun = [&](Int_t first, Int_t last) {
        TString firstStr = Form("%04d", first);
        TString lastStr  = Form("%04d", last);

        if (!result.IsNull())
            result += "+";

        if (first == last)
            result += firstStr;
        else
            result += firstStr + "-" + lastStr;
    };

    for (size_t i = 1; i < runs.size(); ++i) {
        if (runs[i] == prev + 1) {
            // Continue the current range
            prev = runs[i];
        } else {
            // End current range
            appendRun(start, prev);
            start = prev = runs[i];
        }
    }

    // Append the final range
    appendRun(start, prev);

    return result;
}
#endif
