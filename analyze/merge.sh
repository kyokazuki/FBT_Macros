### CONFIG ###
FBT_DATA_DIR=/mnt/daq_data/beamtime2/data1
FBT_RUN_NUM=$1	# input just the digits (not 0001, just 1)
VME_LOGIN="daq@192.168.253.231"
VME_SCRIPT_DIR=/home/daq/exp/2512cyric/artemis

SHARED_DIR_FBT=/mnt/daq_data/daq_shared
SHARED_DIR_VME=/mnt/daq_shared

### MERGE ###
FBT_run_num_formatted=$(printf '%04d' ${FBT_RUN_NUM})
FBT_data=$(ls ${FBT_DATA_DIR}/${FBT_run_num_formatted}*_ext.root)
VME_run_num=$(grep 'Run number' ${SHARED_DIR_FBT}/logs/${FBT_run_num_formatted}_babicon.log | awk 'END { print $4 }')
VME_run_num_formatted=$(printf '%04d' ${VME_run_num})
VME_data=${SHARED_DIR_FBT}/run${VME_run_num_formatted}.root

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
grouped_data=${FBT_data/.root/_grouped2.root}

cd ${FBT_DATA_DIR}
ssh ${VME_LOGIN} \
	"cd ${VME_SCRIPT_DIR} && "\
	"./auto_conv.sh ${VME_run_num} && "\
	"cp -a root/run${VME_run_num_formatted}.root ${SHARED_DIR_VME}"
cp -a ${SHARED_DIR_FBT}/run${VME_run_num_formatted}.root ${FBT_DATA_DIR}
echo "Converting and friending now..."
root -l <<EOF
	.L ${script_dir}/tree_group_events_2.C++g
	treeGroupEvents2("${FBT_data}")
	.L ${script_dir}/tree_friend_2.C++g
	treeFriend("${grouped_data}", "run${VME_run_num_formatted}.root")
	.q
EOF
