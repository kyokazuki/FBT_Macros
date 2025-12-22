### CALIBRATION ###
export DAQ_DIR=/mnt/daq_data/beamtime2
./daqd  --socket-name=/tmp/d.sock --daq-type=GBE
cp config.ini $DAQ_DIR/
./make_bias_calibration_table -o $DAQ_DIR/bias_calibration.tsv
./make_simple_bias_settings_table --config $DAQ_DIR/config.ini --offset 0.0 --prebd 41.44 --bd 51.8 --over 2.5 -o $DAQ_DIR/bias_settings.tsv
./make_simple_channel_map -o $DAQ_DIR/map
sh ../configuration.template.sh $DAQ_DIR
cp ~/FBT_Macros/other/config.ini $DAQ_DIR/
cp ~/FBT_Macros/other/bias_settings.tsv $DAQ_DIR/
cp ~/FBT_Macros/other/map_channel.tsv $DAQ_DIR/
cp ~/FBT_Macros/other/map_trigger.tsv $DAQ_DIR/

### AQUISITION ###
export DAQ_RESULTS_DIR=tot_vs_qdc
./make_simple_disc_settings_table --config $DAQ_DIR/config.ini --vth_t1 10 --vth_t2 10 --vth_e 5 -o $DAQ_DIR/disc_settings.tsv
./read_temperature_sensors --startup
./acquire_sipm_data --config $DAQ_DIR/config.ini --mode qdc --time 1 -o $DAQ_DIR/$DAQ_RESULTS_DIR/test
./acquire_sipm_data_ext --config $DAQ_DIR/config.ini --mode tot --time 1 -o $DAQ_DIR/$DAQ_RESULTS_DIR/test
./convert_raw_to_singles --config $DAQ_DIR/config.ini -i $DAQ_DIR/$DAQ_RESULTS_DIR/test -o $DAQ_DIR/$DAQ_RESULTS_DIR/test_singles.root --writeRoot
./convert_raw_to_group --config $DAQ_DIR/config.ini -i $DAQ_DIR/$DAQ_RESULTS_DIR/test -o $DAQ_DIR/$DAQ_RESULTS_DIR/test_group.root --writeMultipleHits 64 --writeRoot

### ANALYSIS ###
TTree *tree = (TTree*) gDirectory->Get("data")
tree->GetEntries("xi==0 && energy==5")
tree->Scan("*", "", "colsize=20")
tree->Draw("xi", "xi==0 && energy==5")
tree->Draw("time", "xi==0 && energy==5")
tree->Draw("xi", "xi==0 && yi==63")
tree->Draw("tot:xi>>(320, 0.5, 320.5, 100, 0, 200000)", "yi==0", "colz")
events->Draw("totX:urq>>(200, 0, 4500, 200, 0, 300000)", "", "colz")
events->Draw("totX:pow(urq*ulq*drq*dlq, 0.25)>>(200, 0, 4500, 200, 0, 300000)", "urq<4000 && ulq<4000 && drq<4000 && dlq<4000", "colz")
events->Draw("pow(urq*ulq*drq*dlq, 0.25)", "", "colz")
.L ~/FBT_Macros/analyse/plot_time_diff_gate.C+
hdt_fall->Integral(24001, 24120)
.L ~/FBT_Macros/analyse/tree_group_events_2.C+
treeGroupEvents2()
.L ~/FBT_Macros/analyse/tree_friend_2.C+
events->Scan("bbtime:dtime:dbbtime","","colsize=20")
events->Scan("timeX:timeY:timeU:bbtime:dtime:dbbtime","","colsize=20")
events->Draw("(dtime+1)/(dbbtime+1):bbtime", "", "colz")

### MISC ###
# fit poisson
TF1 *f1 = new TF1("f1","[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp(-([1]/[2])))/TMath::Gamma((x/[2])+1.)", 50000, 400000);
f1->SetParameters(100, 14, 250./14)
slice_py_of_->Fit("f1", "R")
fi->Draw("same")

