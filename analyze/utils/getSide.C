#ifndef GETSIDE_C
#define GETSIDE_C

#include <array>
#include <TMath.h>

#include "constants.C"

// even side: xi = 33~64, 97~128, ...
Bool_t isEvenSide(Int_t xi) {
	return !(((xi + 32 - (xi % 32)) / 32) % 2);
}

// FBT center as 0
// odd side (0): y = xiY
// even side (1): y = 224 - xiY
Int_t getDistance(const array<Int_t, 2>& xi, Int_t layer) {
	return (xi[!layer] - LAYER_CHANNELS[!layer] / 2) * TMath::Power(-1, isEvenSide(xi[layer]));
}

#endif
