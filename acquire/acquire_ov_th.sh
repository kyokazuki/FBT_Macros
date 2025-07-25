### CONFIG ###
BUILD_DIR=/home/daq/sw_daq_tofpet2-2024.08.12/build

DAQ_DIR=~/daq_setup6/beamtime  # calibration, bias, mapping etc. files directory
DATA_DIR=FBT	# data directory (appended to DAQ_DIR)
DATA_NAME=beam  # data file name appended to run number

# ASIC_ENUM=($(seq 0 1 15))	# change every ASIC's OV
ASIC_ENUM=(0 1 2 3 5 6 7 8 9 12 13)  # change selected ASIC's OV
# OV_ENUM=($(seq 2.0 0.2 2.6))
OV_ENUM=(3.4)
# TH_ENUM=($(seq 20 1 60))
TH_ENUM=(20)

TIME=900    # aquisition time in secconds
EXT=1	# external gate enabled if 1  (w/o ext gate if 0)

VME=1	# VME starts if 1 (w/o if 0), need tmux session from init_tmux.sh
VME_PADDING=10	# time in seconds of which VME starts after and ends before FBT
VME_DATA_DIR=/mnt/daq_shared	# nfs directory for logs and run sheet

### ACQUIRE ###
if [[ ${VME} -eq 1 ]]; then
	if [[ $(echo "${VME_PADDING}*2" | bc) -ge ${TIME} ]]; then
		echo "VME delay longer than acquisition time!"
		exit 1
	fi
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd ${BUILD_DIR}
for ov in "${OV_ENUM[@]}"; do

	for asic in "${ASIC_ENUM[@]}"; do
		python ${script_dir}/utils/change_ov.py ${DAQ_DIR}/bias_settings.tsv ${asic} ${ov}
	done

	for th in "${TH_ENUM[@]}"; do
		./make_simple_disc_settings_table \
			--config $DAQ_DIR/config.ini \
			--vth_t1 ${th} --vth_t2 0 --vth_e 0 \
			-o $DAQ_DIR/disc_settings.tsv

		run_number=$(python ${script_dir}/utils/get_run_num.py ${DAQ_DIR}/${DATA_DIR})
		file_name="${run_number}_${DATA_NAME}_ov${ov}_th${th}_${TIME}s"
		acquire_script="acquire_sipm_data"
		if [ $EXT -eq 1 ]; then
			file_name+="_ext"
			acquire_script+="_ext"
		fi

		{
			./${acquire_script} \
				--config $DAQ_DIR/config.ini \
				--mode tot --time ${TIME} \
				-o $DAQ_DIR/$DATA_DIR/${file_name} && \
			./convert_raw_to_singles \
				--config $DAQ_DIR/config.ini \
				-i $DAQ_DIR/$DATA_DIR/${file_name} \
				-o $DAQ_DIR/$DATA_DIR/${file_name}.root \
				--writeRoot
		} &
		pid=$!

		if [[ $VME -eq 1 ]]; then
			tmux send-keys -t daq:0.2 'babicon | tee '${VME_DATA_DIR}'/'${run_number}'_babicon.log' C-m
			sleep $(echo "${VME_PADDING} + 2" | bc)
			tmux send-keys -t daq:0.2 'start' C-m
			sleep $(echo "${TIME} - ${VME_PADDING}*2" | bc)
			tmux send-keys -t daq:0.2 'stop' C-m
			tmux send-keys -t daq:0.2 '(automated) FBT run'${run_number} C-m
			tmux send-keys -t daq:0.2 'exit' C-m
			tmux send-keys -t daq:0.2 \
				"echo -e '"${run_number}\
				"\t'\$(printf '%04d\n' \$(grep 'Run number' "${VME_DATA_DIR}"/"${run_number}"_babicon.log | awk 'END { print \$4 }')) >> "\
				${VME_DATA_DIR}"/run_sheet" C-m
		fi

		wait $pid
		sleep 3
	done
done
