#ifndef CONSTANTS_C
#define CONSTANTS_C

// FBT
const char				LAYER_NAMES[3] 			= {'X', 'Y', 'U'};
const Int_t				LAYER_CHANNELS[3]		= {320, 224, 320};
const Double_t			LAYER_COORDS[3]			= {-10.0, -10 + 0.75, 10 - 0.75 / 2};
const Int_t				GATE_CHANNEL_ID			= 4128;

const Int_t				MAX_XI_BINS				= 320;
const Int_t				MAX_XI_RANGE[2]			= {1, 320};
const Int_t				MAX_TIMING_BINS			= 5000; // 1 bin = 1 ns
const Long64_t			MAX_TIMING_RANGE[2]		= {-5000000, 0};
const Int_t				MAX_TOT_BINS			= 500; // 1 bin = 2 ns
const Float_t			MAX_TOT_RANGE[2]		= {0, 1e6};
const Float_t			MAX_TOT_RANGE_SCALED[2]	= {0, 10};

// (xiX + xiY) - (xiU - POS_OFFSET) / POS_SLOPE
// (xiX + xiY) - (xiU + 39.14285) / 0.73142
const Double_t 			POS_SLOPE				= 0.73142;
const Double_t 			POS_OFFSET				= -39.14285;
const Float_t 			MAX_POS_RANGE[2]		= {-600, 600};

// U fiber function: y = -x + ((xiU - 160 - 0.5) * (-0.75) + U_OFFSET) * sqrt(2)
const Double_t 			U_OFFSET				= 0.19153;

// processing
// slewOffset = SLEW_A * exp(SLEW_B * tot) + SLEW_C
// timeCorrected = time - slewOffset
const Double_t			SLEW_A					= 5066;
const Double_t			SLEW_B					= -2.708e-5;
const Double_t			SLEW_C					= -494.6;
const Float_t			TOT_SCALE_TARGET		= 1;

// Hodoscope
const Int_t				HODO_MAX_ID_BINS		= 41;
const Int_t				HODO_MAX_ID_RANGE[2]	= {0, 40}; // 24 empty
const Int_t				HODO_MAX_T_BINS			= 500;
const Double_t			HODO_MAX_T_RANGE[2]		= {0, 1500};
const Int_t				HODO_MAX_Q_BINS			= 500;
const Double_t			HODO_MAX_Q_RANGE[2]		= {0, 10};

// main daq
const vector<Int_t>		ALL_COINS				= {1, 2, 3, 4, 5, 6, 7};

#endif
