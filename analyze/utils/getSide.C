#ifndef GETSIDE_C
#define GETSIDE_C

#include <array>
#include <TMath.h>

#include "constants.C"

// odd side: xi = 1~32, 65~96, ...
Bool_t isOddSide(Int_t xi) {
	// make 1~32 -> 0~31
	xi--;
	// make 0~31 -> 0
	xi -= xi % 32;
	// make 0, 64, ... -> (even number)
	xi = xi / 32;
	// return 1 if is even number
	return !(xi % 2);
}

Double_t xiToPos(Int_t xi, Int_t layer) {
	return -0.75 * ((Double_t)xi - (Double_t)LAYER_CHANNELS[layer] / 2 - 0.5);
}

Double_t xiUToIntercept(Int_t xi) {
	return (((Double_t)xi - 160. - 0.5) * (-0.75) + U_OFFSET_LOCAL) * sqrt(2);
}

// odd side (0): distance = (xiY - 112) * 0.75 * (-1)
// even side (1): distance = (xiY - 112) * 0.75
// mode 0: get X distance from Y, xi = {xiX, xiY, 0}
// mode 1: get Y distance from X, xi = {xiX, xiY, 0}
// mode 2: get U distance from X, xi = {xiX, 0, xiU}
Double_t getDistance(const array<Int_t, 3>& xi, Int_t layer) {
	Double_t odd = TMath::Power(-1, isOddSide(xi[layer]));

	if (layer == 0 || layer == 1) {
		return xiToPos(xi[!layer], !layer) * odd;
	} else {
		Double_t intercept = xiUToIntercept(xi[layer]);
		Double_t slope = -1.0;
		Double_t xPos = xiToPos(xi[0], 0);
		Double_t hitCoord[2] = {xPos, slope * xPos + intercept};
		// calculate the distance from hitCoord to y = x
		return (hitCoord[0] - hitCoord[1]) / sqrt(2) * odd;
	}
}

#endif
