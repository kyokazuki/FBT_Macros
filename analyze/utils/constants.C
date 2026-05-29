#ifndef CONSTANTS_C
#define CONSTANTS_C

// FBT
const char				LAYERS[3] 				= {'X', 'Y', 'U'};
const vector<Int_t>		LAYER_CHANNELS			= {320, 224, 320};
const Int_t				GATE_CHANNEL_ID			= 4128;
const Int_t				MAX_XI_BINS				= 320;
const vector<Int_t>		MAX_XI_RANGE			= {1, 320};
const Int_t				MAX_DT_BINS				= 5000;
const vector<Long64_t>	MAX_DT_RANGE			= {-5000000, 0};
const Int_t				MAX_TOT_BINS			= 500;
const vector<Float_t>	MAX_TOT_RANGE			= {0, 1e6};
const vector<Float_t>	MAX_TOT_RANGE_SCALED	= {0, 10};

const Float_t 			POS_SLOPE				= 0.73142;
const Float_t 			POS_INTERCEPT			= -39.14285;

const Float_t 			U_ANGLE					= 45;
const Float_t 			U_OFFSET				= -39.14285;

// processing
const Float_t			TOT_SCALE_TARGET		= 1;

// Hodoscope
const Int_t				HODO_MAX_ID_BINS		= 40;
const vector<Int_t>		HODO_MAX_ID_RANGE		= {1, 40};
const Int_t				HODO_MAX_Q_BINS			= 500;
const vector<Double_t>	HODO_MAX_Q_RANGE		= {0, 10};

// main daq
const vector<Int_t>		ALL_COINS				= {1, 2, 3, 4, 5, 6, 7};

#endif
