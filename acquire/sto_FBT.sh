#!/bin/sh

source /home/daq/FBT_Macros/acquire/env_FBT.sh

# stop acquisition
printf '\0' > ${DAQ_DIR}/${DATA_DIR}/stop_fifo
rm ${DAQ_DIR}/${DATA_DIR}/start_fifo
rm ${DAQ_DIR}/${DATA_DIR}/stop_fifo

