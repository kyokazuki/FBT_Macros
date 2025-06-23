export DAQ_DIR=~/daq_setup1/S10362-11-100P
export DAQ_DIR=~/daq_setup1/S13360-1375PE
export DAQ_DIR=~/daq_setup2/S13360-1350PE
./daqd  --socket-name=/tmp/d.sock --daq-type=GBE
# create basic configuration files
cp config.ini $DAQ_DIR/
./make_bias_calibration_table -o $DAQ_DIR/bias_calibration.tsv
./make_simple_bias_settings_table --config $DAQ_DIR/config.ini --offset 0.75 --prebd 56.0 --bd 70.0 --over 1.34 -o $DAQ_DIR/bias_settings.tsv
./make_simple_bias_settings_table --config $DAQ_DIR/config.ini --offset 0.0 --prebd 41.46 --bd 51.83 --over 3.0 -o $DAQ_DIR/bias_settings.tsv
./make_simple_channel_map -o $DAQ_DIR/map
cp ~/macro/map_channel.tsv $DAQ_DIR/
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

