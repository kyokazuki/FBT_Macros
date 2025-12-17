export DAQ_DIR=~/daq_setup9/beamtime_final
./daqd  --socket-name=/tmp/d.sock --daq-type=GBE
# create basic configuration files
cp config.ini $DAQ_DIR/
./make_bias_calibration_table -o $DAQ_DIR/bias_calibration.tsv
./make_simple_bias_settings_table --config $DAQ_DIR/config.ini --offset 0.0 --prebd 41.44 --bd 51.8 --over 2.5 -o $DAQ_DIR/bias_settings.tsv
./make_simple_channel_map -o $DAQ_DIR/map
# ASIC calibration
sh ../configuration.template.sh $DAQ_DIR
# set threshold values and acquire data
./make_simple_disc_settings_table --config $DAQ_DIR/config.ini --vth_t1 10 --vth_t2 10 --vth_e 5 -o $DAQ_DIR/disc_settings.tsv
./read_temperature_sensors --startup

export DAQ_RESULTS_DIR=tot_vs_qdc
./acquire_sipm_data --config $DAQ_DIR/config.ini --mode qdc --time 1 -o $DAQ_DIR/$DAQ_RESULTS_DIR/test
./acquire_sipm_data_ext --config $DAQ_DIR/config.ini --mode tot --time 1 -o $DAQ_DIR/$DAQ_RESULTS_DIR/test
./convert_raw_to_singles --config $DAQ_DIR/config.ini -i $DAQ_DIR/$DAQ_RESULTS_DIR/test -o $DAQ_DIR/$DAQ_RESULTS_DIR/test_singles.root --writeRoot
./convert_raw_to_group --config $DAQ_DIR/config.ini -i $DAQ_DIR/$DAQ_RESULTS_DIR/test -o $DAQ_DIR/$DAQ_RESULTS_DIR/test_group.root --writeMultipleHits 64 --writeRoot

# root commands
TTree *tree = (TTree*) gDirectory->Get("data")
tree->Scan()
tree->Draw("energy:channelID>>h(64,-0.5,63.5,480,-2,4)", "", "colz")
tree->Draw("energy:xi>>(64,-0.5,63.5,480,-2,4)", "", "colz")
tree->Draw("tot:xi>>(64,-0.5,63.5,600,-150000,450000)", "", "colz")
tree->Draw("yi:xi>>h(64,-0.5,63.5,200,-2,18)", "", "colz")
.L ~/macro/multiplicity_new.C
.L ~/macro/multiplicity_new.C+
multiplicity("test.root")

# fit poisson
TF1 *f1 = new TF1("f1","[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp(-([1]/[2])))/TMath::Gamma((x/[2])+1.)", 50000, 400000);
f1->SetParameters(100, 14, 250./14)
slice_py_of_->Fit("f1", "R")
fi->Draw("same")

# Beamtime
TTree *tree = (TTree*) gDirectory->Get("data")
tree->GetEntries()
tree->Draw("xi", "xi==0 && energy==5")
tree->Draw("xi", "xi==0 && yi==63")
tree->Draw("tot:xi>>(320, 0.5, 320.5, 100, 0, 900000)", "yi==0", "colz")
tree->Draw("tot:xi>>(320, 0.5, 320.5, 100, 0, 900000)", "yi==1", "colz")
tree->Draw("tot:xi>>(320, 0.5, 320.5, 100, 0, 2000000)", "yi==2", "colz")
events->Draw("totX:urq>>(200, 0, 4500, 200, 0, 300000)", "", "colz")
events->Draw("totX:pow(urq*ulq*drq*dlq, 0.25)>>(200, 0, 4500, 200, 0, 300000)", "urq<4000 && ulq<4000 && drq<4000 && dlq<4000", "colz")
events->Draw("pow(urq*ulq*drq*dlq, 0.25)", "", "colz")
.L ~/FBT_Macros/analyse/get_effcy_merged.C+

