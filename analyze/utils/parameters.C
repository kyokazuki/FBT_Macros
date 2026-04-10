#include <vector>
#include <TString.h>

// GLOBAL
const char* LAYERS[3] 			= {"X","Y","U"};
const Int_t LAYER_CHANNELS[3] 	= {320, 224, 320};
const Float_t TOT_SCALE_TARGET 	= 120e3;

// const Long64_t DT_RANGE[2] 		= {-285000, -250000};	// deuterium
// const Long64_t DT_RANGE[2] 		= {-295000, -270000};	// He3
const Long64_t DT_RANGE[2] 		= {-3880000, -3840000};	// 2512 RARiS

const Float_t TOT_RANGE[2] 		= {20e3, 250e3};	// samurai2506
// const Float_t TOT_RANGE[2] 	= {50e3, 180e3};	// raris2512

// RARiS 2512
const vector<TString> DATASET1 	= {
	"0009_10k_ov2.8_th20_60s_ext_scaled_grouped_friended_rated.root",
	"0028_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0029_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0030_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
};
const vector<TString> DATASET2 	= {
	"0031_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0032_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0033_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
};
const vector<TString> DATASET3 	= {
	"0034_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root",
	"0035_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0036_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0037_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0038_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0040_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0041_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0042_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0044_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0046_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0047_1M_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0048_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0049_1M_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root"
};

