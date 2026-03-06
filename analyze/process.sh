### CONFIG ###
# SAMURAI 2505
# FBT_DIR=/mnt/data1/daq_setup6/beamtime/FBT
# FBT_RUN_ENUM=(103 104 105 115 111 112 119 120 121 129 130 137)	# Z=1 th20 effcy
# FBT_RUN_ENUM=(149 150 151 152 153 154 155 156 157 160 167 169 172)	# Z=1 th40 effcy
# FBT_RUN_ENUM=(502 503 504 505 506 507 508 509 510 511 512 514 515 516 517 518 519 520 523 527)	# Z=2 th20 effcy
# FBT_RUN_ENUM=(520)
# SCALE_DATA="0192"
# LOG_DIR=/mnt/data1/daq_shared/FBT/
# VME_GATEWAY="jiang@lambda.phys.tohoku.ac.jp"	# leave empty for direct connection
# VME_LOGIN="daq@xdaq"
# VME_DIR=/home/daq/exp/2505samurai/artemis2_off
# OFFSET="{0, 1}"
# OFFSET="{$(seq -s ", " 230130 230189)}"

# RARiS 2512
FBT_DIR=/mnt/daq_data/beamtime2/data1
# FBT_RUN_ENUM=($(seq 3 1 57))
FBT_RUN_ENUM=(28)
SCALE_DATA="0006"
LOG_DIR=/mnt/daq_data/daq_shared/logs
VME_GATEWAY="jiang@lambda.phys.tohoku.ac.jp"	# leave empty for direct connection
VME_LOGIN="daq@172.25.28.98"
VME_DIR=/home/daq/exp/2512cyric/artemis

SCALE=0
GROUP=0
VME=0
FRIEND_OFFSET=0
FRIEND_DELTA=0
CLEAN_CROSSTALK=1
CLEAN_GEOMETRY=0
RATE=0

### MERGE ###
scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd ${FBT_DIR}

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
	cleanedCData=${friendedData/.root/_cleanedC.root}
	cleanedGData=${cleanedCData/.root/_cleanedG.root}
	echo "Processing ${rootData}"

	# scale
	if [[ ${SCALE} -eq 1 ]]; then
		root -l -b -q "${scriptDir}/processScale.C+(\"${rootData}\", \"${SCALE_DATA}\")"
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
	if [[ ${VME} -eq 1 || ${FRIEND_DELTA} -eq 1 || ${FRIEND_OFFSET} -eq 1 ]]; then
		if [[ ! -e ${LOG_DIR}/${fbtRunNumber}_babicon.log ]]; then
			echo "Cannot find ${LOG_DIR}/${fbtRunNumber}_babicon.log!"
			vmeRun=0
		else
			echo "Found ${LOG_DIR}/${fbtRunNumber}_babicon.log"
			vmeRun=$(grep 'Run number' ${LOG_DIR}/${fbtRunNumber}_babicon.log | awk 'END { print $4 }')
			vmeRunNumber=$(printf '%04d' ${vmeRun})
		fi
	fi

	# login to VME machine and get the root file
	if [[ ${VME} -eq 1 && ${vmeRun} -ne 0 ]]; then
		ssh -J ${VME_GATEWAY} ${VME_LOGIN} \
			"cd ${VME_DIR} && "\
			"./auto_conv.sh ${vmeRun}"
		if [ -z ${VME_GATEWAY} ]; then
			scp ${VME_LOGIN}:${VME_DIR}/root/run${vmeRunNumber}.root ${FBT_DIR}
		else
			scp -J ${VME_GATEWAY} ${VME_LOGIN}:${VME_DIR}/root/run${vmeRunNumber}.root ${FBT_DIR}
			# scp -J ${VME_GATEWAY} ${VME_LOGIN}:${VME_DIR}/root.m/run${vmeRunNumber}.m.root ${FBT_DIR}
		fi
	fi

	# friend if both files exist (simple)
	if [[ ${FRIEND_OFFSET} -eq 1 && ${vmeRun} -ne 0 ]]; then
		if [[ ! -e ${groupedData} ]]; then
			echo "Cannot find ${groupedData}!"
		elif [[ ! -e run${vmeRunNumber}.m.root ]]; then
			echo "Cannot find run${vmeRunNumber}.m.root!"
		else
			root -l -b -q "${scriptDir}/processFriendOffset.C+(\"${groupedData}\", \"run${vmeRunNumber}.m.root\", ${OFFSET})"
		fi
	fi
	# friend if both files exist
	if [[ ${FRIEND_DELTA} -eq 1 && ${vmeRun} -ne 0 ]]; then
		if [[ ! -e ${groupedData} ]]; then
			echo "Cannot find ${groupedData}!"
		elif [[ ! -e run${vmeRunNumber}.root ]]; then
			echo "Cannot find run${vmeRunNumber}.root!"
		else
			root -l -b -q "${scriptDir}/processFriendDelta.C+(\"${groupedData}\", \"run${vmeRunNumber}.root\")" | tee ${fbtRunNumber}_friend.log
		fi
	fi

	# clean if file exists
	if [[ ${CLEAN_CROSSTALK} -eq 1 ]]; then
		if [[ ! -e ${friendedData} ]]; then
			echo "Cannot find ${friendedData}!"
		else
			root -l -b -q "${scriptDir}/processCleanCrosstalk.C+(\"${friendedData}\")"
		fi
	fi

	# clean if file exists
	if [[ ${CLEAN_GEOMETRY} -eq 1 ]]; then
		if [[ ! -e ${cleanedCData} ]]; then
			echo "Cannot find ${cleanedCData}!"
		else
			root -l -b -q "${scriptDir}/processCleanGeometry.C+(\"${cleanedCData}\")"
		fi
	fi

	# rate if file exists
	# if [[ ${RATE} -eq 1 ]]; then
	# 	if [[ ! -e ${cleanedGData} ]]; then
	# 		echo "Cannot find ${cleanedGData}!"
	# 	else
	# 		root -l -b -q "${scriptDir}/processRate.C+(\"${cleanedGData}\")"
	# 	fi
	# fi
	if [[ ${RATE} -eq 1 ]]; then
		if [[ ! -e ${friendedData} ]]; then
			echo "Cannot find ${friendedData}!"
		else
			root -l -b -q "${scriptDir}/processRate.C+(\"${friendedData}\")"
		fi
	fi

done
