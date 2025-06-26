### CONFIG ###
FBT_DATA_DIR=~/daq_setup6/beamtime/FBT
FBT_RUN_NUM=$1
VME_LOGIN="daq@10.32.19.169"
VME_SCRIPT_DIR=/home/daq/exp/2505samurai/artemis2_off

SHARED_DIR_FBT=/mnt/data1/daq_shared
SHARED_DIR_VME=/mnt/daq_shared

### MERGE ###
FBT_run_num_formatted=$(printf '%04d' ${FBT_RUN_NUM})
FBT_data=$(ls ${FBT_DATA_DIR}/${FBT_run_num_formatted}*_ext.root)
VME_run_num=$(grep 'Run number' ${SHARED_DIR_FBT}/${FBT_run_num_formatted}_babicon.log | awk 'END { print $4 }')
VME_run_num_formatted=$(printf '%04d' ${VME_run_num})
VME_data=${SHARED_DIR_FBT}/run${VME_run_num_formatted}.m.root

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
grouped_data=${FBT_data/.root/_grouped2.root}

ssh ${VME_LOGIN} \
	"cd ${VME_SCRIPT_DIR} && "\
	"./auto_conv.sh ${VME_run_num} && "\
	"./auto_sca.sh ${VME_run_num} && "\
	"cp -a root.m/run${VME_run_num_formatted}.m.root ${SHARED_DIR_VME}"
root -l <<EOF
	.L ${script_dir}/tree_group_events_2.C+
	treeGroupEvents2("${FBT_data}")
	.L ${script_dir}/tree_friend.C+
	treeFriend("${grouped_data}", "${VME_data}")
	.q
EOF
