### CONFIG ###
FBT_DATA_DIR=/mnt/daq_data/beamtime2/data1
FBT_RUN_NUM_ENUM=($(seq 3 1 57))	# input just the digits (not 0001, just 1)
# FBT_RUN_NUM_ENUM=(43)
# FBT_RUN_NUM_ENUM=(39)
# FBT_RUN_NUM_ENUM=(29)

GROUP=0
VME=0
FRIEND=0
CLEAN=1

SHARED_DIR_FBT=/mnt/daq_data/daq_shared

# VME_LOGIN="daq@192.168.253.231"
VME_LOGIN="daq@xdaq01"
VME_SCRIPT_DIR=/home/daq/exp/2512cyric/artemis
SHARED_DIR_VME=/mnt/daq_shared

### MERGE ###
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd ${FBT_DATA_DIR}

for fbt_run_num in "${FBT_RUN_NUM_ENUM[@]}"; do
	fbt_run_num_formatted=$(printf '%04d' ${fbt_run_num})
	echo "================================"$fbt_run_num_formatted"================================"

	# check if root file exists
	root_data=("$FBT_DATA_DIR"/"$fbt_run_num_formatted"*_ext.root)
	if [[ ! -e "${root_data[0]}" || -e "${root_data[1]}" ]]; then
		echo "Found 0 or more than 1 "$fbt_run_num_formatted"*_ext.root file!"
		continue
	fi
	echo "Found exactly 1 "$fbt_run_num_formatted"*_ext.root file"

	root_data="${root_data[0]}"
	grouped_data=${root_data/.root/_grouped.root}
	friended_data=${grouped_data/.root/_friended.root}

	# group root file
	if [[ ${GROUP} -eq 1 ]]; then
		echo "Grouping now..."
		root -l -b -q "${script_dir}/treeGroup.C+(\"${root_data}\")"
	fi

	# get responding VME run number
	if [[ ! -e ${SHARED_DIR_FBT}/logs/${fbt_run_num_formatted}_babicon.log ]]; then
		echo "Cannot find ${SHARED_DIR_FBT}/logs/${fbt_run_num_formatted}_babicon.log!"
	else
		echo "Found ${SHARED_DIR_FBT}/logs/${fbt_run_num_formatted}_babicon.log"
		vme_run_num=$(grep 'Run number' ${SHARED_DIR_FBT}/logs/${fbt_run_num_formatted}_babicon.log | awk 'END { print $4 }')
		vme_run_num_formatted=$(printf '%04d' ${vme_run_num})

		# login to VME machine and get the root file
		if [[ ${VME} -eq 1 ]]; then
			ssh ${VME_LOGIN} \
				"cd ${VME_SCRIPT_DIR} && "\
				"./auto_conv.sh ${vme_run_num} && "\
				"cp -a root/run${vme_run_num_formatted}.root ${SHARED_DIR_VME}"
			cp -a ${SHARED_DIR_FBT}/run${vme_run_num_formatted}.root ${FBT_DATA_DIR}
		fi

		# friend only if files exist
		if [[ ${FRIEND} -eq 1 ]]; then
			if [[ ! -e ${grouped_data} ]]; then
				echo "Cannot find ${grouped_data}!"
			elif [[ ! -e run${vme_run_num_formatted}.root ]]; then
				echo "Cannot find run${vme_run_num_formatted}.root!"
			else
				echo "Friending now..."
				root -l -b -q "${script_dir}/treeFriendDelta.C+(\"${grouped_data}\", \"run${vme_run_num_formatted}.root\")"  \
					| tee ${fbt_run_num_formatted}_friendDelta.log
			fi
		fi
	fi

	# clean only if file exists
	if [[ ${CLEAN} -eq 1 ]]; then
		if [[ ! -e ${friended_data} ]]; then
			echo "Cannot find ${friended_data}!"
		else
			echo "Cleaning now..."
			root -l -b -q "${script_dir}/treeClean.C+(\"${friended_data}\")"
		fi
	fi
done
