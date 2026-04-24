#!/bin/sh

# daq variables
source /home/daq/FBT_Macros/acquire/env_FBT.sh

# get data file and acquisition script names
RUN_NUM=$1 # in 4 digits
file_name="${RUN_NUM}_${DATA_NAME}_ov${OV}_th${TH}_${TIME}s"
echo "Acquiring for ${file_name}"
acquire_script="acquire_sipm_data"
if [ ${EXT} -eq 1 ]; then
	file_name+="_ext"
	acquire_script+="_ext"
fi

# set overvoltages
for asic in $(seq 0 1 13); do
	python \
		/home/daq/FBT_Macros/acquire/utils/changeOV.py \
		${DAQ_DIR}/bias_settings.tsv \
		${asic} ${OV}
done

# set thresholds
${BUILD_DIR}/make_simple_disc_settings_table \
	--config ${DAQ_DIR}/config.ini \
	--vth_t1 ${TH} --vth_t2 0 --vth_e 0 \
	-o ${DAQ_DIR}/disc_settings.tsv

# acquisition standby
mkfifo ${DAQ_DIR}/${DATA_DIR}/start_fifo
mkfifo ${DAQ_DIR}/${DATA_DIR}/stop_fifo
${BUILD_DIR}/${acquire_script} \
	--config ${DAQ_DIR}/config.ini \
	--mode tot \
	--time ${TIME} \
	-o ${DAQ_DIR}/${DATA_DIR}/${file_name} \
	--wait-on ${DAQ_DIR}/${DATA_DIR}/start_fifo \
	--stop-on ${DAQ_DIR}/${DATA_DIR}/stop_fifo \
	| tee ${DAQ_DIR}/${DATA_DIR}/${file_name}.log
# ${BUILD_DIR}/${acquire_script} \
# --config ${DAQ_DIR}/config.ini \
# --mode tot \
# --time ${TIME} \
# -o ${DAQ_DIR}/${DATA_DIR}/${file_name} \
# | tee ${DAQ_DIR}/${DATA_DIR}/${file_name}.log

# rename time to live time
if [ ${RENAME} -eq 1 ]; then
	start_time=$(stat -c %Y ${DAQ_DIR}/${DATA_DIR}/start_fifo)
	stop_time=$(stat -c %Y ${DAQ_DIR}/${DATA_DIR}/stop_fifo)
	live_time=$(( stop_time - start_time ))
	new_file_name="${RUN_NUM}_${DATA_NAME}_ov${OV}_th${TH}_${live_time}s"
	if [ ${EXT} -eq 1 ]; then
		new_file_name+="_ext"
	fi
	rename "${file_name}" "${new_file_name}" ${DAQ_DIR}/${DATA_DIR}/${file_name}.*
	file_name=${new_file_name}
fi

# cleanup
rm ${DAQ_DIR}/${DATA_DIR}/start_fifo
rm ${DAQ_DIR}/${DATA_DIR}/stop_fifo

# convert and copy in background
{
	if [ ${CONVERT} -eq 1 ]; then
		# create converting. file while converting because runs in background
		touch ${DAQ_DIR}/${DATA_DIR}/converting.${file_name}.root
		${BUILD_DIR}/convert_raw_to_singles \
			--config ${DAQ_DIR}/config.ini \
			-i ${DAQ_DIR}/${DATA_DIR}/${file_name} \
			-o ${DAQ_DIR}/${DATA_DIR}/${file_name}.root \
			--writeRoot \
			2>&1 | tee -a ${DAQ_DIR}/${DATA_DIR}/${file_name}.log &&
		rm ${DAQ_DIR}/${DATA_DIR}/converting.${file_name}.root
	fi

	if [ ${COPY} -eq 1 ]; then
		rsync -a -e "ssh -i ${COPY_KEY}" ${DAQ_DIR}/${DATA_DIR}/${file_name}.* ${COPY_DEST}
	fi
} &
disown

