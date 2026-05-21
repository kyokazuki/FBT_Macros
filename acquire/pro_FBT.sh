#!/bin/sh

# process variables
FBT_DIR=/home/s074/fbt/FDC1_test
RUN_NUM=$1
MACRO_DIR=/home/s074/fbt/FBT_Macros
DT_RANGE="{-3880000, -3840000}" # time range relative to gate for grouping [ps]
FRIEND_DATA=/home/s074/fdc1/run$1.root

GROUP=0
FRIEND=0

# get root file names
rootData=(${FBT_DIR}/${RUN_NUM}*_ext.root)
if [[ ! -e "${rootData[0]}" ]]; then
	echo "Cannot find run ${RUN_NUM}!"
	exit 1
fi
rootData="${rootData[0]}"

# group data
if [[ ${GROUP} -eq 1 ]]; then
	if [[ ! -e ${rootData} ]]; then
		echo "Cannot find ${rootData}!"
		exit 1
	else
		root -l -b -q "${MACRO_DIR}/processGroup.C+(\"${rootData}\", ${DT_RANGE})"
	fi
	groupedData=${rootData/.root/_grouped.root}
fi

# friend data
if [[ ${FRIEND} -eq 1 ]]; then
	if [[ ! -e ${groupedData} ]]; then
		echo "Cannot find ${groupedData}!"
		exit 1
	elif [[ ! -e ${FRIEND_DATA} ]]; then
		echo "Cannot find ${FRIEND_DATA}!"
		exit 1
	else
		root -l -b -q "${MACRO_DIR}/processFriendFDC1.C+(\"${groupedData}\", \"${FRIEND_DATA}\", 0)"
	fi
fi

