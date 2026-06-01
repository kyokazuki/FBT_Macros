#ifndef ZOOMAXIS_C
#define ZOOMAXIS_C

#include <vector>
#include <algorithm>
#include <limits>

#include <TH1.h>

template <typename HistT>
void zoomAxisX(const vector<HistT*>& hists, Int_t padMinus = 0, Int_t padPlus = 0) {
    Int_t firstBin = std::numeric_limits<Int_t>::max();
    Int_t lastBin  = std::numeric_limits<Int_t>::min();

    for (const HistT* hist : hists) {
        Int_t first = hist->FindFirstBinAbove(0.0, 1);
        Int_t last  = hist->FindLastBinAbove(0.0, 1);

		// skip if empty
        if (first <= 0 || last <= 0) continue;

        firstBin = std::min(firstBin, first);
        lastBin  = std::max(lastBin, last);
    }

	if (firstBin == std::numeric_limits<Int_t>::max()) return;

    for (HistT* hist : hists) {
        Int_t maxBin = hist->GetNbinsX();
        hist->GetXaxis()->SetRange(
            std::max(1, firstBin - padMinus),
            std::min(maxBin, lastBin + padPlus)
        );
    }
}

template <typename HistT>
void zoomAxisY(const vector<HistT*>& hists, Int_t padMinus = 0, Int_t padPlus = 0) {
    Int_t firstBin = std::numeric_limits<Int_t>::max();
    Int_t lastBin  = std::numeric_limits<Int_t>::min();

    for (const HistT* hist : hists) {
        Int_t first = hist->FindFirstBinAbove(0.0, 2);
        Int_t last  = hist->FindLastBinAbove(0.0, 2);

		// skip if empty
        if (first <= 0 || last <= 0) continue;

        firstBin = std::min(firstBin, first);
        lastBin  = std::max(lastBin, last);
    }

	if (firstBin == std::numeric_limits<Int_t>::max()) return;

    for (HistT* hist : hists) {
        Int_t maxBin = hist->GetNbinsY();
        hist->GetYaxis()->SetRange(
            std::max(1, firstBin - padMinus),
            std::min(maxBin, lastBin + padPlus)
        );
    }
}

template <typename HistT>
void zoomAxisAll(const vector<HistT*>& hists, Int_t padMinus = 0, Int_t padPlus = 0) {
	zoomAxisX(hists, padMinus, padPlus);
	zoomAxisY(hists, padMinus, padPlus);
}

template <typename HistT>
void zoomAxisX(HistT* hist, Int_t padMinus = 0, Int_t padPlus = 0) {
    zoomAxisX(vector<HistT*>{hist}, padMinus, padPlus);
}

template <typename HistT>
void zoomAxisY(HistT* hist, Int_t padMinus = 0, Int_t padPlus = 0) {
    zoomAxisY(vector<HistT*>{hist}, padMinus, padPlus);
}

template <typename HistT>
void zoomAxisAll(HistT* hist, Int_t padMinus = 0, Int_t padPlus = 0) {
	zoomAxisX(hist, padMinus, padPlus);
	zoomAxisY(hist, padMinus, padPlus);
}

#endif
