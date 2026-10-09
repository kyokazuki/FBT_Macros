#!/bin/sh

# parameters
BUILD_DIR=/home/daq/sw_daq_tofpet2-2026.03.20/build_disc # software directory
DAQ_DIR=/home/daq/daq_setup11/beamtime # calibration directory
DATA_DIR=data1
TARGET_DATA_DIR=remapped
# TARGET_DATA_DIR=remapped/tot_cali
# TARGET_DATA_DIR=remapped/time_cali
# TARGET_DATA_DIR=remapped/effcy

# RUN_NUM=($(seq -f "%04g" ${1:-0000} 1 ${2:-0000}))
# RUN_NUM=($(seq -f "%04g" 1071 1 1075))
# RUN_NUM=($(seq -f "%04g" 1071 1075) 1192 1193 1195 $(seq -f "%04g" 1200 1203))
RUN_NUM=(1352)
# RUN_NUM=($(seq -f "%04g" 1070 1075)) # tot_cali
# RUN_NUM=(1211 1213 1215 1217 1218 1219 1221 1222 1224 $(seq -f "%04g" 1226 1234)) # time_cali
# RUN_NUM=(1029 1031 1035 1037 1040 1041 1043 1044 1045 1049 1051 1052) # effcy

CONVERT=0
GROUP=0
GROUP_TIMING="{-3540000, -3490000}"
SELECT=0
GATE=0
COIN=0
SLEW=1
SHIFT=1
SHIFT_RUN="/home/daq/daq_setup11/beamtime/remapped/time_cali/1211+1213+1215+1217-1219+1221-1222+1224+1226-1234_timeOffsetsCoined"
SCALE=1
SCALE_RUN="/home/daq/daq_setup11/beamtime/remapped/tot_cali/1070-1075_totMeans"
PAD=0
HODO=0

ANALYZE_SINGLES=0
ANALYZE_SINGLES_SCALED=0
ANALYZE_GROUPED=0
ANALYZE_GROUPED_SCALED=0

for runNum in "${RUN_NUM[@]}"; do
	echo "Processing run ${runNum}"

	# get root file names
	rawPath=(${DAQ_DIR}/${DATA_DIR}/${runNum}*.rawf)
	if (( ${#rawPath[@]} != 1 )) || [[ ! -e "${rawPath[0]}" ]]; then
		echo "Cannot find run ${runNum}!"
		continue
	fi
	rawPath="${rawPath[0]}"
	rawFile=$(basename "${rawPath}")
	dataName="${rawFile%.*}"

	if [ ${CONVERT} -eq 1 ]; then
		${BUILD_DIR}/convert_raw_to_singles \
			--config ${DAQ_DIR}/config.ini \
			-i ${DAQ_DIR}/${DATA_DIR}/${dataName} \
			-o ${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}.root \
			--writeRoot
	fi
	if [ ${GROUP} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processGroup.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}.root\", ${GROUP_TIMING})"
	fi
	if [ ${SELECT} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processSelect.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped.root\")"
	fi
	if [ ${GATE} -eq 1 ]; then
		# root -l -b -q \
		# 	"/home/daq/FBT_Macros/analyze/processGate.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_selected.root\")"
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processGate.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped.root\")"
	fi
	if [ ${COIN} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processFriendCoin.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_gated.root\", \"${DAQ_DIR}/${TARGET_DATA_DIR}/hodo${runNum}.root\")"
	fi
	if [ ${SLEW} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processSlew.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_gated_coined.root\")"
		# root -l -b -q \
		# 	"/home/daq/FBT_Macros/analyze/processGate.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped.root\")"
	fi
	if [ ${SHIFT} -eq 1 ]; then
		# root -l -b -q \
		# 	"/home/daq/FBT_Macros/analyze/processShift.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped.root\", \"${SHIFT_RUN}\")"
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processShift.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_gated_coined_slewed2.root\", \"${SHIFT_RUN}\")"
	fi
	if [ ${SCALE} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processScale.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_gated_coined_slewed2_shifted.root\", \"${SCALE_RUN}\")"
		# root -l -b -q \
		# 	"/home/daq/FBT_Macros/analyze/processScale.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_gated_coined.root\", \"${SCALE_RUN}\")"
	fi
	if [ ${PAD} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processPad.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_scaled.root\", \"${DAQ_DIR}/${TARGET_DATA_DIR}/smts${runNum}.root\")"
	fi
	if [ ${HODO} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processFriendHodo.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_scaled_padded.root\", \"${DAQ_DIR}/${TARGET_DATA_DIR}/hodo${runNum}.root\")"
	fi

	if [ ${ANALYZE_SINGLES} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/analyzeSingles.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}.root\")"
	fi
	if [ ${ANALYZE_SINGLES_SCALED} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/analyzeSinglesScaled.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_scaled.root\")"
	fi
	if [ ${ANALYZE_GROUPED} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/analyzeGrouped.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped.root\")"
	fi
	if [ ${ANALYZE_GROUPED_SCALED} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/analyzeGroupedScaled.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_shifted_coined_scaled.root\")"
	fi
done

