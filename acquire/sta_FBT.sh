#!/bin/sh

source /home/daq/FBT_Macros/acquire/env_FBT.sh

# start acquisition
printf '\0' > ${DAQ_DIR}/${DATA_DIR}/start_fifo

