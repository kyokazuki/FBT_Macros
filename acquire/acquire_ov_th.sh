SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DAQ_DIR=~/daq_setup5/beamtime  # calibration, bias, mapping etc. files directory
DATA_DIR=data	# appended to DAQ_DIR
DATA_NAME=test  # data file prefix

TIME=300    # in unit of sec
EXT=1	# external gate enabled if 1  (wo ext gate if 0)

# ASICs=($(seq 0 15))	# change every ASIC's OV
ASICs=(0 1 2 5 6 7 8 9 12 13)  # change selected ASIC's OV
OFFSET=0.0
BD=51.80
OV_MIN=3.2    # change here for change HV
OV_MAX=3.2    # change here for change HV
OV_INCREMENT=0.2 # change here for change HV

TH_MIN=40   # change here for change TH
TH_MAX=40   # change here for change TH
TH_INCREMENT=1  # change here for change TH

for ov in $(seq $OV_MIN $OV_INCREMENT $OV_MAX); do

	for asic in "${ASICs[@]}"; do
		python ${SCRIPT_DIR}/change_ov.py ${DAQ_DIR}/bias_settings.tsv ${asic} ${ov}
	done

	for th in $(seq $TH_MIN $TH_INCREMENT $TH_MAX); do
		./make_simple_disc_settings_table \
			--config $DAQ_DIR/config.ini \
			--vth_t1 ${th} --vth_t2 0 --vth_e 0 \
			-o $DAQ_DIR/disc_settings.tsv

		if [ $EXT -eq 1 ]; then
			file_name="${DATA_NAME}_ov${ov}_th${th}_${TIME}s_ext"
			./acquire_sipm_data_ext \
				--config $DAQ_DIR/config.ini \
				--mode tot --time ${TIME} \
				-o $DAQ_DIR/$DATA_DIR/${file_name}
		else
			file_name="${DATA_NAME}_ov${ov}_th${th}_${TIME}s"
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
