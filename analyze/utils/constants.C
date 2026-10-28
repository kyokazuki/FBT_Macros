#ifndef CONSTANTS_C
#define CONSTANTS_C

// FBT
const char			LAYER_NAMES[3] 				= {'X', 'Y', 'U'};
const Int_t			LAYER_CHANNELS[3]			= {320, 224, 320};
const Double_t		LAYER_COORDS[3]				= {-10.0, -10 + 0.75, 10 - 0.75 / 2};
const Int_t			GATE_CHANNEL_ID				= 4128;

const Int_t			MAX_XI_BINS					= 320;
const Int_t			MAX_XI_RANGE[2]				= {1, 320};
const Int_t			MAX_TIMING_BINS				= 5000; // 1 bin = 1 ns
const Long64_t		MAX_TIMING_RANGE[2]			= {-5000000, 0};
const Int_t			MAX_TOT_BINS				= 500; // 1 bin = 2 ns
const Float_t		MAX_TOT_RANGE[2]			= {0, 1e6};
const Float_t		MAX_TOT_RANGE_SCALED[2]		= {0, 10};

// (xiX + xiY) - (xiU - POS_OFFSET) / POS_SLOPE
// (xiX + xiY) - (xiU + 39.14285) / 0.73142
const Double_t		MISALIGNMENT_SLOPE			= 0.73142;
const Double_t		MISALIGNMENT_OFFSET			= -39.14285;
const Float_t 		MAX_MISALIGNMENT_RANGE[2]	= {-600, 600};

// U fiber function: y = -x + ((xiU - 160 - 0.5) * (-0.75) + U_OFFSET) * sqrt(2)
const Double_t 		U_OFFSET_LOCAL				= 0.19153 + 0.225;

// processing
// slewOffset = SLEW_A * exp(SLEW_B * tot) + SLEW_C
// timeCorrected = time - slewOffset
// const Double_t		SLEW_A[3]					= {4598.80, 4441.42, 3958.31};
// const Double_t		SLEW_B[3]					= {-2.06046e-5, -1.31647e-5, -1.85376e-5};
// const Double_t		SLEW_C[3]					= {-3.52492e6, -3.52181e6, -3.52348e6};
const Double_t		SLEW_A[3]					= {4567, 0, 0};
const Double_t		SLEW_B[3]					= {-2.02684e-5, 0, 0};
const Double_t		SLEW_C[3]					= {-3.52494e6, 0, 0};
const Float_t		TOT_SCALE_TARGET			= 1;
const Double_t		PROPAGATION_SLOPE			= 5.625;
const Double_t		PROPAGATION_INTERCEPT		= 31.470;

// Hodoscope
const Int_t			HODO_MAX_ID_BINS			= 41;
const Int_t			HODO_MAX_ID_RANGE[2]		= {0, 40}; // 24 empty
const Int_t			HODO_MAX_T_BINS				= 500;
const Double_t		HODO_MAX_T_RANGE[2]			= {0, 1500};
const Int_t			HODO_MAX_Q_BINS				= 500;
const Double_t		HODO_MAX_Q_RANGE[2]			= {0, 10};

// main daq
const vector<Int_t>	ALL_COINS					= {1, 2, 3, 4, 5, 6, 7};

// S074 geometry
// const Int_t 		TOGAXSI_WINDOW_XI[2][2]		= {{80, 230}, {43, 193}}; // xiX, xiY, xiU range for TOGAXSI window
const Double_t 		TOGAXSI_WINDOW_CENTER[3]	= {0, 0, 588.2}; // TOGAXSI window center coord
const Double_t 		TOGAXSI_WINDOW_WIDTH		= 70; // TOGAXSI window width [mm]
const Double_t 		TARGET_CENTER[3]			= {-2.5, -4.0, 56.5}; // coords from togaxsi entrance [mm]
const Double_t 		TARGET_RADIUS				= 17.5;
const Double_t 		FBT_CENTER[3]				= {4.451, 0.208, 889.8}; // coords from togaxsi entrance [mm]
const Double_t 		U_OFFSET_GLOBAL				= U_OFFSET_LOCAL + (FBT_CENTER[0] + FBT_CENTER[1]) / sqrt(2);

#endif
