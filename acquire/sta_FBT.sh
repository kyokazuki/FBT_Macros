#!/bin/sh

# daq variables
source /home/daq/FBT_Macros/acquire/env_FBT.sh

# start acquisition
if timeout 1 bash -c "printf '\0' > ${DAQ_DIR}/${DATA_DIR}/start_fifo"; then
	date "+%Y/%m/%d %H:%M:%S : start_fifo write succeeded"
else
    echo "start_fifo write blocked!"
fi

