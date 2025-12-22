### CONFIG ###
BUILD_DIR=/home/daq/sw_daq_tofpet2-2025.08.04/build

# DAQ_DIR=~/daq_setup9/beamtime_final  # calibration, bias, mappin166312g etc. files directory
DAQ_DIR=/mnt/daq_data/beamtime2  # calibration, bias, mappin166312g etc. files directory
DATA_DIR=data1	# data directory (appended to DAQ_DIR)
DATA_NAME=Sr90 # data file name appended to run number (usually source name)

ASIC_ENUM=($(seq 0 1 13))	# change every ASIC's OV
# ASIC_ENUM=(0 1 2 5 6 7 8 9 12 13)  # change selected ASIC's OV
# OV_ENUM=($(seq 1.0 0.1 1.4))
OV_ENUM=(2.8)
#TH_ENUM=($(seq 10 1 50))
TH_ENUM=(20)

TIME=60	#aquisition time in secconds
EXT=0	# external gate enabled if 1  (w/o ext gate if 0)

VME=0	# VME starts if 1 (w/o if 0), need tmux session from init_tmux.sh

VME_PADDING=15	# time in seconds of which VME starts later and ends earlier than FBT
VME_LOG_DIR=/mnt/daq_shared/logs	# nfs directory for logs and run sheet

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
			--vth_t1 ${th} --vth_t2 1 --vth_e 1 \
			-o $DAQ_DIR/disc_settings.tsv

		run_number=$(python ${script_dir}/utils/get_run_num.py ${DAQ_DIR}/${DATA_DIR})
		file_name="${run_number}_${DATA_NAME}_ov${ov}_th${th}_${TIME}s"
		acquire_script="acquire_sipm_data"
		# acquire_script="acquire_sipm_data_debug"
		if [ $EXT -eq 1 ]; then
			file_name+="_ext"
			acquire_script+="_ext"
		fi
		if [ $VME -eq 1 ]; then
			mkfifo $DAQ_DIR/$DATA_DIR/start_fifo
			acquire_script+=" --wait-on $DAQ_DIR/$DATA_DIR/start_fifo"
		fi
		{
			./${acquire_script} \
				--config $DAQ_DIR/config.ini \
				--mode tot --time ${TIME} \
				-o $DAQ_DIR/$DATA_DIR/${file_name} && \
				echo "Acquired ${DAQ_DIR}/${DATA_DIR}/${file_name}"
		} &
		pid=$!
		if [[ $VME -eq 1 ]]; then
			sleep 10
			tmux send-keys -t daq:0.2 'babicon | tee '${VME_LOG_DIR}'/'${run_number}'_babicon.log' C-m
			tmux send-keys -t daq:0.2 'wth' C-m
			tmux send-keys -t daq:0.2 '(automated) FBT '$DAQ_DIR'/'$DATA_DIR'/'${file_name} C-m
			printf '\0' > $DAQ_DIR/$DATA_DIR/start_fifo
			sleep $(echo "${VME_PADDING}" | bc)
			tmux send-keys -t daq:0.2 'start' C-m
			sleep $(echo "${TIME} - ${VME_PADDING}*2" | bc)
			tmux send-keys -t daq:0.2 'stop' C-m
			tmux send-keys -t daq:0.2 '(automated) FBT '$DAQ_DIR'/'$DATA_DIR'/'${file_name} C-m
			tmux send-keys -t daq:0.2 'exit' C-m
			tmux send-keys -t daq:0.2 \
				"echo -e '"${run_number}\
				"\t'\$(printf '%04d\n' \$(grep 'Run number' "${VME_LOG_DIR}"/"${run_number}"_babicon.log | awk 'END { print \$4 }')) >> "\
				${VME_LOG_DIR}"/run_sheet" C-m
			rm $DAQ_DIR/$DATA_DIR/start_fifo
		fi
		wait $pid
		{
			touch $DAQ_DIR/$DATA_DIR/converting.${file_name}.root
			./convert_raw_to_singles \
				--config $DAQ_DIR/config.ini \
				-i $DAQ_DIR/$DATA_DIR/${file_name} \
				-o $DAQ_DIR/$DATA_DIR/${file_name}.root \
				--writeRoot && \
			rm $DAQ_DIR/$DATA_DIR/converting.${file_name}.root
			echo "Converted ${DAQ_DIR}/${DATA_DIR}/${file_name}"
		} &
		sleep 3
	done
done
