# DAQ_DIR=~/daq_setup1/S10362-11-100P
# OFFSET=0.75
# BD=70.0
# OV_MIN=0.0
# OV_MAX=2.4
# OV_INCREMENT=0.2

# DAQ_DIR=~/daq_setup1/S13360-1375PE
# OFFSET=0.75
# BD=48.0
# OV_MIN=4.0
# OV_MAX=8.0
# OV_INCREMENT=0.2

# DAQ_DIR=~/daq_setup1/S13360-1350PE
# OFFSET=0.0
# BD=51.83
# OV_MIN=2.0
# OV_MAX=2.0
# OV_INCREMENT=1.0

# # DAQ_DIR=~/daq_setup2/S13360-1350PE_uncalibrated
# DAQ_DIR=~/daq_setup2/S13360-1350PE
# default BD=51.83 OV=3

DAQ_DIR=~/daq_setup5/beamtime  # place where calibration, bias, mapping etc. files

ASICs=(0 1 2 5 6 7 8 9 12 13)  # ASIC numbers for new mppc (change HVs only for those MPPCs)
OFFSET=0.0
BD=51.80
OV_MIN=3.2    # change here for change HV
OV_MAX=3.2    # change here for change HV
OV_INCREMENT=0.2 # change here for change HV

# DATA_DIR=heavy
#DATA_DIR=light  # appended to DAQ_DIR
DATA_DIR=light2  # appended to DAQ_DIR
# DATA_DIR=th-rate
# DATA_DIR=ov-effcy

# RAD_SOURCE=GIS-4
#RAD_SOURCE=Both-7  # name for data 
RAD_SOURCE=Both-35  # name for data 
# RAD_SOURCE=Sr90
# RAD_SOURCE=LED1k

TH_MIN=40   # change here for change TH
TH_MAX=40   # change here for change TH
TH_INCREMENT=1  # change here for change TH
TIME=300    # in unit of sec
EXT=1 # external gate enabled if 1  (wo ext gate if 0)
#EXT=0 # external gate enabled if 1  (wo ext gate if 0)

for ov in $(seq $OV_MIN $OV_INCREMENT $OV_MAX); do
	# ./make_simple_bias_settings_table \
	# 	--config $DAQ_DIR/config.ini \
	# 	--offset ${OFFSET} --prebd $(echo "$BD * 0.8" | bc) --bd $BD --over $ov \
	# 	-o $DAQ_DIR/bias_settings.tsv

	for asic in "${ASICs[@]}"; do
		python ~/macro/change_ov.py ${DAQ_DIR}/bias_settings.tsv ${asic} ${ov}
	done

	for th in $(seq $TH_MIN $TH_INCREMENT $TH_MAX); do
		./make_simple_disc_settings_table \
			--config $DAQ_DIR/config.ini \
			--vth_t1 ${th} --vth_t2 0 --vth_e 0 \
			-o $DAQ_DIR/disc_settings.tsv

		if [ $EXT -eq 1 ]; then
			file_name="${RAD_SOURCE}_ov${ov}_th${th}_${TIME}s_ext"
			./acquire_sipm_data_ext \
				--config $DAQ_DIR/config.ini \
				--mode tot --time ${TIME} \
				-o $DAQ_DIR/$DATA_DIR/${file_name}
		else
			file_name="${RAD_SOURCE}_ov${ov}_th${th}_${TIME}s"
			./acquire_sipm_data \
				--config $DAQ_DIR/config.ini \
				--mode tot --time ${TIME} \
				-o $DAQ_DIR/$DATA_DIR/${file_name}
		fi

		./convert_raw_to_singles \
			--config $DAQ_DIR/config.ini \
			-i $DAQ_DIR/$DATA_DIR/${file_name} \
			-o $DAQ_DIR/$DATA_DIR/${file_name}.root \
			--writeRoot &
	done
done
