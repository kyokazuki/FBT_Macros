### CALIBRATION ###
export DAQ_DIR=/home/daq/daq_setup11/FDC1_test
./daqd  --socket-name=/tmp/d.sock --daq-type=GBE
cp config.ini $DAQ_DIR/
./make_bias_calibration_table -o $DAQ_DIR/bias_calibration.tsv
./make_simple_bias_settings_table --config $DAQ_DIR/config.ini --offset 0.0 --prebd 41.44 --bd 51.8 --over 2.5 -o $DAQ_DIR/bias_settings.tsv
./make_simple_channel_map -o $DAQ_DIR/map
sh ../configuration.template.sh $DAQ_DIR
sh ../configuration.template.sh $DAQ_DIR --enable-dark-scans
cp ~/FBT_Macros/other/config.ini $DAQ_DIR/
cp ~/FBT_Macros/other/bias_settings.tsv $DAQ_DIR/
cp ~/FBT_Macros/other/map_channel.tsv $DAQ_DIR/
cp ~/FBT_Macros/other/map_trigger.tsv $DAQ_DIR/

### AQUISITION ###
export DAQ_RESULTS_DIR=test_speed
./make_simple_disc_settings_table --config $DAQ_DIR/config.ini --vth_t1 20 --vth_t2 0 --vth_e 0 -o $DAQ_DIR/disc_settings.tsv
./read_temperature_sensors --startup
mkfifo $DAQ_DIR/$DAQ_RESULTS_DIR/start_fifo
mkfifo $DAQ_DIR/$DAQ_RESULTS_DIR/stop_fifo
printf '\0' > $DAQ_DIR/$DATA_DIR/start_fifo
printf '\0' > $DAQ_DIR/$DATA_DIR/stop_fifo
./acquire_sipm_data --config $DAQ_DIR/config.ini --mode tot --time 1 -o $DAQ_DIR/$DAQ_RESULTS_DIR/test
./acquire_sipm_data_ext --config $DAQ_DIR/config.ini --mode tot --time 600 -o $DAQ_DIR/$DAQ_RESULTS_DIR/bg
./acquire_sipm_data_ext --config $DAQ_DIR/config.ini --mode tot --time 600 -o $DAQ_DIR/$DAQ_RESULTS_DIR/bg --wait-on $DAQ_DIR/$DAQ_RESULTS_DIR/start_fifo --stop-on $DAQ_DIR/$DAQ_RESULTS_DIR/stop_fifo

./convert_raw_to_singles --config $DAQ_DIR/config.ini -i $DAQ_DIR/$DAQ_RESULTS_DIR/test -o $DAQ_DIR/$DAQ_RESULTS_DIR/test_singles.root --writeRoot
./convert_raw_to_group --config $DAQ_DIR/config.ini -i $DAQ_DIR/$DAQ_RESULTS_DIR/test -o $DAQ_DIR/$DAQ_RESULTS_DIR/test_group.root --writeMultipleHits 64 --writeRoot

### ANALYSIS ###
TTree *tree = (TTree*) gDirectory->Get("data")
tree->GetEntries("channelID==4128 && energy==5")
tree->Draw("tot:xi>>(320, 0.5, 320.5, 100, 0, 200000)", "yi==0 && channelID!=4128", "colz")
tree->Draw("xi>>(320, 0.5, 320.5)", "yi==0 && channelID!=4128")
events->Scan("timeX:energyX:totX:channelIDX:xiX:timeY:energyY:totY:channelIDY:xiY:timeU:energyU:totU:channelIDU:xiU:timeGate", "", "colsize=10")
events->Scan("timeX:totX:xiX:timeY:totY:xiY:timeU:totU:xiU:timeGate", "", "colsize=10")

## hodo
tree->Draw("totX:xiX>>(320, 0.5, 320.5, 100, 0, 10)", "", "colz")
tree->Draw("fQCal:fID>>(40, -0.5, 39.5, 200, 0, 40)", "", "colz")

## bdc
tree->Draw("TargetX:TargetY>>(300, -150, 150, 300, -150, 150)", "", "colz")
tree->Draw("TargetA:TargetB>>(500, -0.08, 0.08, 500, -0.08, 0.08)", "", "colz")

events->Draw("totX:urq>>(200, 0, 4500, 200, 0, 300000)", "", "colz")
events->Draw("totX:pow(urq*ulq*drq*dlq, 0.25)>>(200, 0, 4500, 200, 0, 300000)", "urq<4000 && ulq<4000 && drq<4000 && dlq<4000", "colz")
events->Draw("pow(urq*ulq*drq*dlq, 0.25)", "", "colz")
hdt_fall->Integral(24001, 24120)
events->Scan("bbtime:dtime:dbbtime","","colsize=20")
events->Scan("timeX:timeY:timeU:bbtime:dtime:dbbtime","","colsize=20")
events->Draw("(dtime+1)/(dbbtime+1):bbtime", "", "colz")

### MISC ###
# (delay[ns] = value * 5 – 2)
# ./pas_fbt
# fit poisson
TF1 *f1 = new TF1("f1","[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp(-([1]/[2])))/TMath::Gamma((x/[2])+1.)", 50000, 400000);
f1->SetParameters(100, 14, 250./14)
slice_py_of_->Fit("f1", "R")
fi->Draw("same")

plotRateTot({"0028_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", "0029_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", "0030_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", "0031_200k_ov2.8_th20_300s_ext_scaled_grouped_friended_rated.root", "0032_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", "0033_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root", "0034_200k_ov2.8_th20_1800s_ext_scaled_grouped_friended_rated.root"})

### UPDATE FIRMWARE ###
./update_prom --port 0 --slave 0 --bin feb_d2_gbe_fem128n_2023.11.27_0000.bit --method alternate

# add friend
TFile *f1 = TFile::Open("file1.root");
TTree *t1 = (TTree*)f1->Get("tree1");
TFile *f2 = TFile::Open("file2.root");
TTree *t2 = (TTree*)f2->Get("tree2");
t1->AddFriend(t2);

TFile *f1 = TFile::Open("0038_cosmic_ov2.8_th20_86400s_ext_grouped.root");
TFile *f2 = TFile::Open("out.root");
TTree *t1 = (TTree*)f1->Get("events");
TTree *t2 = (TTree*)f2->Get("tree")
t1->AddFriend(t2)
t1->Scan("ts:xiX:xiY:xiU:fdc1.wirepos:fdc1.wirez")




// GLOBAL
const char LAYERS[3] 			= {'X', 'Y', 'U'};
const Int_t LAYER_CHANNELS[3] 	= {320, 224, 320};
const Float_t TOT_SCALE_TARGET 	= 120e3;

// const Long64_t DT_RANGE[2] 		= {-285000, -250000};	// deuterium
// const Long64_t DT_RANGE[2] 		= {-295000, -270000};	// He3
const Long64_t DT_RANGE[2] 		= {-3880000, -3840000};	// 2512 RARiS

// const Float_t TOT_RANGE[2] 		= {20e3, 250e3};	// samurai2506
const Float_t TOT_RANGE[2] 	= {50e3, 180e3};	// raris2512

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

