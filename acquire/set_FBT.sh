#!/bin/sh

source /home/daq/FBT_Macros/acquire/env_FBT.sh

# set overvoltages
for asic in $(seq 0 1 13); do
	python \
		/home/daq/FBT_Macros/acquire/utils/changeOV.py \
		${DAQ_DIR}/bias_settings.tsv \
		${asic} ${ov}
done

# set thresholds
${BUILD_DIR}/make_simple_disc_settings_table \
	--config ${DAQ_DIR}/config.ini \
	--vth_t1 ${TH} --vth_t2 0 --vth_e 0 \
	-o ${DAQ_DIR}/disc_settings.tsv

# get data file and acquisition script names
file_name="${RUN_NUM}_${DATA_NAME}_ov${ov}_th${th}_${TIME}s"
acquire_script="acquire_sipm_data"
if [ ${EXT} -eq 1 ]; then
	file_name+="_ext"
	acquire_script+="_ext"
fi

# acquisition standby
mkfifo ${DAQ_DIR}/${DATA_DIR}/start_fifo
mkfifo ${DAQ_DIR}/${DATA_DIR}/stop_fifo
${BUILD_DIR}/${acquire_script} \
	--config ${DAQ_DIR}/config.ini \
	--mode tot \
	--time ${TIME} \
	-o ${DAQ_DIR}/${DATA_DIR}/${file_name} \
	--wait-on ${DAQ_DIR}/${DATA_DIR}/start_fifo
	--stop-on ${DAQ_DIR}/${DATA_DIR}/stop_fifo
sleep 5

