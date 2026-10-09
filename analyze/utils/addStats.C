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
void addStats(T* obj, const std::vector<TString>& values) {
    if constexpr (std::is_base_of_v<TH1, T>)
        obj->SetStats(false);

    auto* stat = new TPaveStats(
        0.65,
        0.92 - (values.size() + 1) * 0.03,
        0.90,
        0.92,
        "NDC"
    );

    stat->SetTextFont(42);
    stat->SetTextSize(0.025);

    for (const auto& v : values)
        stat->AddText(v);

    obj->GetListOfFunctions()->Add(stat);
}

template <typename T>
TString vectorToString(const std::vector<T>& v)
{
    std::ostringstream oss;
    oss << "{";

    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0)
            oss << ", ";

        oss << v[i];
    }

    oss << "}";

    return TString(oss.str());
}

#endif
