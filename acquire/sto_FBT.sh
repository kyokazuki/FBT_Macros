#!/bin/sh

# daq variables
source /home/daq/FBT_Macros/acquire/env_FBT.sh

# stop acquisition
if timeout 1 bash -c "printf '\0' > ${DAQ_DIR}/${DATA_DIR}/stop_fifo"; then
    echo "stop_fifo write succeeded"
else
    echo "stop_fifo write blocked!"
fi

