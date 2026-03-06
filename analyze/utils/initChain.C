#include <TChain.h>
#include <vector>

const vector <string> inputFiles1 	= {
	"0028_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0029_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0030_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0031_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", 
	"0032_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
	"0033_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", 
};
const vector <string> inputFiles2 	= {
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

TChain* inputChain = nullptr;

void initChain(const vector <string>& inputFiles, const string& treeName) {
	// initialize inputChain
	delete inputChain;
	inputChain = new TChain(treeName.c_str());
	for (const string& file : inputFiles) {
		inputChain->Add(file.c_str());
	}
}
