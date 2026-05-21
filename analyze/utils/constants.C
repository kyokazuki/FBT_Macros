#ifndef CONSTANTS_C
#define CONSTANTS_C

// global constants
const char				LAYERS[3] 			= {'X', 'Y', 'U'};
const vector<Int_t>		LAYER_CHANNELS		= {320, 224, 320};
const Int_t				GATE_CHANNEL_ID		= 4128;

// for initializing graphs
const vector<Int_t>		MAX_XI_RANGE		= {1, 320};	// largest possible fiber channel range
const Int_t				MAX_XI_BINS			= 320; // largest amount of xi bins
const vector<Long64_t>	MAX_DT_RANGE		= {-5000000, 0}; // largest possible gate delay range
const Long64_t			MAX_DT_BINS			= 5000;	// largest amount of gate delay bins
const vector<Long64_t>	MAX_TOT_RANGE		= {0, 1000000};	// largest possible ext gate delay range
const Long64_t			MAX_TOT_BINS		= 500; // largest amount of tot bins

#endif
