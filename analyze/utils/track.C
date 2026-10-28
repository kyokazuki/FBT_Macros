#ifndef TRACK_C
#define TRACK_C

#include "getSide.C"

Double_t xiToPosGlobal(Int_t xi, Int_t layer) {
	return xiToPos(xi, layer) + FBT_CENTER[layer];
}

Double_t xiUToInterceptGlobal(Int_t xi) {
	return (((Double_t)xi - 160. - 0.5) * (-0.75) + U_OFFSET_GLOBAL) * sqrt(2);
}

Double_t getDistanceGlobal(
	const array<array<Double_t, 3>, 3>& coord, 
	const array<Int_t, 3>& xi, 
	Int_t layer
) {
	Double_t odd = TMath::Power(-1, isOddSide(xi[layer]));
	if (layer == 0 || layer == 1) {
		// return getDistance(xi, layer);
		return (coord[layer][!layer] - FBT_CENTER[!layer]) * odd;
	} else {
		return ((coord[layer][0] - FBT_CENTER[0]) - (coord[layer][1] - FBT_CENTER[1])) / sqrt(2) * odd;
	}
}

Double_t getStdDev(Double_t value1, Double_t value2) {
	return abs(value1 - value2) / 2;
}

Double_t getStdDev(Double_t value1, Double_t value2, Double_t value3) {
	Double_t average = (value1 + value2 + value3) / 3;
	Double_t timeSD = 0;
	timeSD += pow((value1 - average), 2);
	timeSD += pow((value2 - average), 2);
	timeSD += pow((value3 - average), 2);
	timeSD = sqrt(timeSD / 3);
	return timeSD;
}

struct Track {
	Int_t projectedLayer, layer1, layer2;
	array<Double_t, 3> reactionCoord;

	array<Int_t, 3> hit = {-1, -1, -1};
	array<Int_t, 3> xi = {0, 0, 0};
	array<array<Double_t, 3>, 3> coord;
	array<Double_t, 2> togaxsiWindowCoord;
	Bool_t inWindow = 0;

	Double_t projectionDiff; // distance of (fiber - projected position)

	array<Double_t, 3> time = {0, 0, 0};
	Double_t timeSD = 0;

	array<Double_t, 3> tot = {0, 0, 0};
	Double_t totAverage = 0;
	Double_t totSD = 0;

	Track(
		Int_t projectedLayer, 
		UInt_t hit1, 
		UInt_t hit2, 
		Int_t xi1, 
		Int_t xi2, 
		const Double_t (&reactionCoord)[3]
	) 
		: reactionCoord({reactionCoord[0], reactionCoord[1], reactionCoord[2]}), projectedLayer(projectedLayer) 
	{
		Double_t reactionDistance = FBT_CENTER[2] - reactionCoord[2]; // mm
		layer1 = (projectedLayer + 1) % 3;
		layer2 = (projectedLayer + 2) % 3;
		hit[layer1] = hit1;
		hit[layer2] = hit2;
		xi[layer1] = xi1;
		xi[layer2] = xi2;

		array<Double_t, 2> pos;
		array<Double_t, 2> tan;
		array<Double_t, 2> posAtU;

		// calculate the position at U first of all, then backtrack using tan to each layer
		if (projectedLayer == 2) {
			for (Int_t layer = 0; layer < 2; layer++) {
				pos[layer] = xiToPosGlobal(xi[layer], layer);
				tan[layer] = (pos[layer] - reactionCoord[layer]) / (reactionDistance + LAYER_COORDS[layer]);
				posAtU[layer] = pos[layer] + (LAYER_COORDS[2] - LAYER_COORDS[layer]) * tan[layer];
			}
		} else {
			Double_t intercept = xiUToInterceptGlobal(xi[2]);
			Double_t slope = -1.0;

			pos[!projectedLayer] = xiToPosGlobal(xi[!projectedLayer], !projectedLayer);
			tan[!projectedLayer] = (pos[!projectedLayer] - reactionCoord[!projectedLayer]) / (reactionDistance + LAYER_COORDS[!projectedLayer]);
			posAtU[!projectedLayer] = pos[!projectedLayer] + (LAYER_COORDS[2] - LAYER_COORDS[!projectedLayer]) * tan[!projectedLayer];
			if (projectedLayer == 0) {
				posAtU[projectedLayer] = (posAtU[!projectedLayer] - intercept) / slope;
			} else if (projectedLayer == 1) {
				posAtU[projectedLayer] = posAtU[!projectedLayer] * slope + intercept;
			}
			tan[projectedLayer] = (posAtU[projectedLayer] - reactionCoord[projectedLayer]) / (reactionDistance + LAYER_COORDS[2]);
		}

		// trace from the position on U to each layer
		for (Int_t layer = 0; layer < 3; layer++) {
			coord[layer][2] = FBT_CENTER[2] + LAYER_COORDS[layer];
			for (Int_t axis = 0; axis < 2; axis++) {
				coord[layer][axis] = posAtU[axis] + (LAYER_COORDS[layer] - LAYER_COORDS[2]) * tan[axis];
			}
		}

		// trace from the position on U to TOGAXSI window
		for (Int_t axis = 0; axis < 2; axis++) {
			togaxsiWindowCoord[axis] = posAtU[axis] + (TOGAXSI_WINDOW_CENTER[2] - (FBT_CENTER[2] + LAYER_COORDS[2])) * tan[axis];
		}
		inWindow = abs(togaxsiWindowCoord[0]) < TOGAXSI_WINDOW_WIDTH / 2 && abs(togaxsiWindowCoord[1]) < TOGAXSI_WINDOW_WIDTH / 2;
	}

	Double_t getProjectionDiff(UInt_t hit3, Int_t xi3) {
		hit[projectedLayer] = hit3;
		xi[projectedLayer] = xi3;

		if (projectedLayer == 2) {
			Double_t intercept = xiUToInterceptGlobal(xi[2]);
			Double_t slope = -1.0;
			projectionDiff = (slope * coord[2][0] - coord[2][1] + intercept) / sqrt(slope * slope + 1.0); 
		} else {
			// for projectedLayer = X, Y
			projectionDiff = xiToPosGlobal(xi[projectedLayer], projectedLayer) - coord[projectedLayer][projectedLayer];
		}
		return projectionDiff;
	}

	// for two-fiber tracks
	Double_t getTimeSD(Long64_t time1, Long64_t time2, Bool_t propagated) {
		time[layer1] = (Double_t)time1;
		time[layer2] = (Double_t)time2;

		// shift propagation time
		if (propagated) {
			for (Int_t layer : {layer1, layer2}) {
				Double_t distance = getDistanceGlobal(coord, xi, layer);
				// time[layer] -= distance * PROPAGATION_SLOPE + PROPAGATION_INTERCEPT;
				time[layer] -= distance * PROPAGATION_SLOPE;
			}
		}

		timeSD = getStdDev(time[layer1], time[layer2]);
		return timeSD;
	}

	// for three-fiber tracks
	Double_t getTimeSD(Long64_t time1, Long64_t time2, Long64_t time3, Bool_t propagated) {
		time[layer1] = (Double_t)time1;
		time[layer2] = (Double_t)time2;
		time[projectedLayer] = (Double_t)time3;

		// shift propagation time
		if (propagated) {
			for (Int_t layer : {layer1, layer2, projectedLayer}) {
				Double_t distance = getDistanceGlobal(coord, xi, layer);
				// time[layer] -= distance * PROPAGATION_SLOPE + PROPAGATION_INTERCEPT;
				time[layer] -= distance * PROPAGATION_SLOPE;
			}
		}

		timeSD = getStdDev(time[layer1], time[layer2], time[projectedLayer]);
		return timeSD;
	}

	Double_t getTotSD(Float_t tot1, Float_t tot2) {
		tot[layer1] = (Double_t)tot1;
		tot[layer2] = (Double_t)tot2;

		totAverage = (tot[layer1] + tot[layer2]) / 2;
		totSD = getStdDev(tot[layer1], tot[layer2]);
		return totSD;
	}

	Double_t getTotSD(Float_t tot1, Float_t tot2, Float_t tot3) {
		tot[layer1] = (Double_t)tot1;
		tot[layer2] = (Double_t)tot2;
		tot[projectedLayer] = (Double_t)tot3;

		totAverage = (tot[layer1] + tot[layer2] + tot[projectedLayer]) / 3;
		totSD = getStdDev(tot[layer1], tot[layer2], tot[projectedLayer]);
		return totSD;
	}
};

#endif
