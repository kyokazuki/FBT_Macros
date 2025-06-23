### CONFIG ###
DAQ_DIR=~/daq_setup6/beamtime-test  # calibration, bias, mapping etc. files directory
DATA_DIR=test2	# data directory (appended to DAQ_DIR)
DATA_NAME=bg  # data file name appended to run number

# ASIC=($(seq 0 15))	# change every ASIC's OV
ASIC=(0 1 2 3 5 6 7 8 9 12 13)  # change selected ASIC's OV
OV_MIN=2.6    # change here for change HV
OV_MAX=2.6    # change here for change HV
OV_INCREMENT=0.2 # change here for change HV

TH_MIN=20   # change here for change TH
TH_MAX=20   # change here for change TH
TH_INCREMENT=1  # change here for change TH

TIME=5    # aquisition time in secconds
EXT=0	# external gate enabled if 1  (wo ext gate if 0)

### ACQUIRE ###
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
for ov in $(seq $OV_MIN $OV_INCREMENT $OV_MAX); do

	for asic in "${ASIC[@]}"; do
		python ${script_dir}/utils/change_ov.py ${DAQ_DIR}/bias_settings.tsv ${asic} ${ov}
	done

	for th in $(seq $TH_MIN $TH_INCREMENT $TH_MAX); do
		./make_simple_disc_settings_table \
			--config $DAQ_DIR/config.ini \
			--vth_t1 ${th} --vth_t2 0 --vth_e 0 \
			-o $DAQ_DIR/disc_settings.tsv

		run_number=$(python ${script_dir}/utils/get_run_num.py ${DAQ_DIR}/${DATA_DIR})
		if [ $EXT -eq 1 ]; then
			file_name="${run_number}_${DATA_NAME}_ov${ov}_th${th}_${TIME}s_ext"
			./acquire_sipm_data_ext \
				--config $DAQ_DIR/config.ini \
				--mode tot --time ${TIME} \
				-o $DAQ_DIR/$DATA_DIR/${file_name}
		else
			file_name="${run_number}_${DATA_NAME}_ov${ov}_th${th}_${TIME}s"
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
