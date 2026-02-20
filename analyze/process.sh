### CONFIG ###
FBT_DATA_DIR=/mnt/daq_data/beamtime2/data1
# FBT_RUN_ENUM=($(seq 3 1 57))	# input just the digits (not 0001, just 1)
# FBT_RUN_ENUM=($(seq 52 1 57))	# input just the digits (not 0001, just 1)
FBT_RUN_ENUM=(52)	# input just the digits (not 0001, just 1)

SCALE=0
GROUP=0
VME=0
FRIEND=1
RATE=1
CLEAN=0

SHARED_DIR_FBT=/mnt/daq_data/daq_shared

VME_GATEWAY="jiang@lambda.phys.tohoku.ac.jp"
# VME_LOGIN="daq@192.168.253.231"
VME_LOGIN="daq@172.25.28.98"
VME_SCRIPT_DIR=/home/daq/exp/2512cyric/artemis
SHARED_DIR_VME=/mnt/daq_shared

### MERGE ###
scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd ${FBT_DATA_DIR}

for fbtRun in "${FBT_RUN_ENUM[@]}"; do
	fbtRunNumber=$(printf '%04d' ${fbtRun})
	echo "=================================${fbtRunNumber}================================="

	# get root file names
	rootData=(${fbtRunNumber}*_ext.root)
	if [[ ! -e "${rootData[0]}" ]]; then
		echo "Cannot find run ${fbtRunNumber}!"
		continue
	fi
	rootData="${rootData[0]}"
	scaledData=${rootData/.root/_scaled.root}
	groupedData=${scaledData/.root/_grouped.root}
	friendedData=${groupedData/.root/_friended.root}
	cleanedData=${friendedData/.root/_cleaned.root}
	echo "Processing ${rootData}"

	# scale
	if [[ ${SCALE} -eq 1 ]]; then
		root -l -b -q "${scriptDir}/processScale.C+(\"${rootData}\", \"0006\")"
	fi

	# group if file exists
	if [[ ${GROUP} -eq 1 ]]; then
		if [[ ! -e ${scaledData} ]]; then
			echo "Cannot find ${scaledData}!"
		else
			root -l -b -q "${scriptDir}/processGroup.C+(\"${scaledData}\")"
		fi
	fi

	# get responding VME run number
	if [[ ${VME} -eq 1 || ${FRIEND} -eq 1 ]]; then
		if [[ ! -e ${SHARED_DIR_FBT}/logs/${fbtRunNumber}_babicon.log ]]; then
			echo "Cannot find ${SHARED_DIR_FBT}/logs/${fbtRunNumber}_babicon.log!"
			vmeRun=0
		else
			echo "Found ${SHARED_DIR_FBT}/logs/${fbtRunNumber}_babicon.log"
			vmeRun=$(grep 'Run number' ${SHARED_DIR_FBT}/logs/${fbtRunNumber}_babicon.log | awk 'END { print $4 }')
			vmeRunNumber=$(printf '%04d' ${vmeRun})
		fi
	fi

	# login to VME machine and get the root file
	if [[ ${VME} -eq 1 && ${vmeRun} -ne 0 ]]; then
		# ssh -J ${VME_GATEWAY} ${VME_LOGIN} \
		# 	"cd ${VME_SCRIPT_DIR} && "\
		# 	"./auto_conv.sh ${vmeRun} && "\
		# 	"cp -a root/run${vmeRunNumber}.root ${SHARED_DIR_VME}"
		# cp -a ${SHARED_DIR_FBT}/run${vmeRunNumber}.root ${FBT_DATA_DIR}
		ssh -J ${VME_GATEWAY} ${VME_LOGIN} \
			"cd ${VME_SCRIPT_DIR} && "\
			"./auto_conv.sh ${vmeRun}"
		scp -J ${VME_GATEWAY} ${VME_LOGIN}:${VME_SCRIPT_DIR}/root/run${vmeRunNumber}.root ${FBT_DATA_DIR}
	fi

	# friend if both files exist
	if [[ ${FRIEND} -eq 1 && ${vmeRun} -ne 0 ]]; then
		if [[ ! -e ${groupedData} ]]; then
			echo "Cannot find ${groupedData}!"
		elif [[ ! -e run${vmeRunNumber}.root ]]; then
			echo "Cannot find run${vmeRunNumber}.root!"
		else
			root -l -b -q "${scriptDir}/processFriend.C+(\"${groupedData}\", \"run${vmeRunNumber}.root\")" | tee ${fbtRunNumber}_friend.log
		fi
	fi

	# rate if file exists
	if [[ ${RATE} -eq 1 ]]; then
		if [[ ! -e ${friendedData} ]]; then
			echo "Cannot find ${friendedData}!"
		else
			root -l -b -q "${scriptDir}/processRate.C+(\"${friendedData}\")"
		fi
	fi

	# clean if file exists
	if [[ ${CLEAN} -eq 1 ]]; then
		if [[ ! -e ${rateedData} ]]; then
			echo "Cannot find ${rateedData}!"
		else
			root -l -b -q "${scriptDir}/processClean.C+(\"${rateedData}\")"
		fi
	fi
done
