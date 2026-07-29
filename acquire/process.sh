#!/bin/sh

# parameters
BUILD_DIR=/home/daq/sw_daq_tofpet2-2026.03.20/build_disc # software directory
DAQ_DIR=/home/daq/daq_setup11/beamtime # calibration directory
DATA_DIR=data1
# TARGET_DATA_DIR=${DATA_DIR}
TARGET_DATA_DIR=remapped
# TARGET_DATA_DIR=remapped/anchor

# RUN_NUM=($(seq -f "%04g" ${1:-0000} 1 ${2:-0000}))
# RUN_NUM=($(seq -f "%04g" 1071 1 1075))
# RUN_NUM=(1192 1193 1195 $(seq -f "%04g" 1200 1203))
# RUN_NUM=($(seq -f "%04g" 1071 1075) 1192 1193 1195 $(seq -f "%04g" 1200 1203))
RUN_NUM=(1287)

CONVERT=0
GROUP=0
GROUP_TIMING="{-3540000, -3490000}"
SHIFT=0
SHIFT_RUN="anchor/1071-1075+1192-1193+1195+1200-1203_timeOffsets"
SCALE=1
SCALE_RUN="anchor/1071-1075_totMeans"

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
	if [ ${SHIFT} -eq 1 ]; then
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processShift.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped.root\", \"${SHIFT_RUN}\")"
	fi
	if [ ${SCALE} -eq 1 ]; then
		# root -l -b -q \
		# 	"/home/daq/FBT_Macros/analyze/processScale.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_shifted_coined.root\", \"${SCALE_RUN}\")"
		root -l -b -q \
			"/home/daq/FBT_Macros/analyze/processScale.C+(\"${DAQ_DIR}/${TARGET_DATA_DIR}/${runNum}_grouped_shifted.root\", \"${SCALE_RUN}\")"
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

