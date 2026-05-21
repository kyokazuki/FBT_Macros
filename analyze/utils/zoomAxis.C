#ifndef ZOOMAXIS_C
#define ZOOMAXIS_C

#include <TH1.h>

void zoomAxisX(TH1* inGraph, Int_t padMinus = 0, Int_t padPlus = 0) {
    inGraph->GetXaxis()->SetRange(
        inGraph->FindFirstBinAbove(0.0, 1) - padMinus,
        inGraph->FindLastBinAbove(0.0, 1) + padPlus
    );
}

void zoomAxisY(TH1* inGraph, Int_t padMinus = 0, Int_t padPlus = 0) {
    inGraph->GetYaxis()->SetRange(
        inGraph->FindFirstBinAbove(0.0, 2) - padMinus,
        inGraph->FindLastBinAbove(0.0, 2) + padPlus
    );
}

#endif
