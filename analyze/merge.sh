### CONFIG ###
FBT_DATA_DIR=/mnt/daq_data/beamtime2/data1
FBT_RUN_NUM_ENUM=($(seq 32 1 57))	# input just the digits (not 0001, just 1)
# FBT_RUN_NUM_ENUM=(33)
SHARED_DIR_FBT=/mnt/daq_data/daq_shared

VME=0
VME_LOGIN="daq@192.168.253.231"
VME_SCRIPT_DIR=/home/daq/exp/2512cyric/artemis
SHARED_DIR_VME=/mnt/daq_shared

### MERGE ###
for fbt_run_num in "${FBT_RUN_NUM_ENUM[@]}"; do

	# {
	fbt_run_num_formatted=$(printf '%04d' ${fbt_run_num})
	fbt_data=$(ls ${FBT_DATA_DIR}/${fbt_run_num_formatted}*_ext.root)
	vme_run_num=$(grep 'Run number' ${SHARED_DIR_FBT}/logs/${fbt_run_num_formatted}_babicon.log | awk 'END { print $4 }')
	vme_run_num_formatted=$(printf '%04d' ${vme_run_num})
	vme_data=${SHARED_DIR_FBT}/run${vme_run_num_formatted}.root

	script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	grouped_data=${fbt_data/.root/_grouped.root}

	cd ${FBT_DATA_DIR}
	if [[ ${VME} -eq 1 ]]; then
	# if [[ ! -f "run${vme_run_num_formatted}.root" ]]; then
		ssh ${VME_LOGIN} \
			"cd ${VME_SCRIPT_DIR} && "\
			"./auto_conv.sh ${vme_run_num} && "\
			"cp -a root/run${vme_run_num_formatted}.root ${SHARED_DIR_VME}"
		cp -a ${SHARED_DIR_FBT}/run${vme_run_num_formatted}.root ${FBT_DATA_DIR}
	fi

	echo "Converting and friending now..."

	root -l -b -q "${script_dir}/treeGroup.C+(\"${fbt_data}\")"
	root -l -b -q "${script_dir}/treeFriendDelta.C+(\"${grouped_data}\", \"run${vme_run_num_formatted}.root\")"
	# } &

done
