#!/bin/sh

# acquisition variables
BUILD_DIR=/home/daq/sw_daq_tofpet2-2026.03.20/build_disc # software directory
DAQ_DIR=/home/daq/daq_setup11/beamtime # calibration directory
DATA_DIR=data1 # data directory (appended to DAQ_DIR)
DATA_NAME=beam # data file name appended to run number
OV=3.4 # overvoltage
TH=20 # threshold
TIME=86400 # acquisition time in seconds (86400: 1 day, 604800: 1 week)
# TIME=10 # acquisition time in seconds (86400: 1 day, 604800: 1 week)
EXT=1 # external gate enabled if 1
RENAME=0 # rename time to live time if 1

# process variables
CONVERT=1 # convert to root if 1
SCALE=0
SCALE_RUN="1074" # run used for scalling
GROUP=1 # group events if 1
GROUP_DT="{-3540000, -3490000}"

# analysis variables
ANALYZE_SINGLES=1 # save analysis graph for rootfile
ANALYZE_SINGLES_SCALED=1 # save analysis graph for rootfile
ANALYZE_GROUPED=1 # save analysis graph for grouped rootfile
ANALYZE_GROUPED_TOT_RANGE="{50e3, 1e6}" # save analysis graph for grouped rootfile
ANALYZE_GROUPED_SCALED=0 # save analysis graph for grouped rootfile
ANALYZE_GROUPED_SCALED_TOT_RANGE="{0.5, 10}" # save analysis graph for grouped rootfile

# copy variables
COPY=1 # copy data if 1
COPY_KEY="/home/daq/.ssh/id_ed25519_sana01" # ssh key to use
COPY_RAW_DEST="s074@sana01.ribfdaq:/home/s074/rawdata/fbt"
COPY_ROOT_DEST="s074@sana01.ribfdaq:/home/s074/rootfiles/fbt"
COPY_FRIEND_DEST="s074@sana01.ribfdaq:/home/s074/rootfiles.friend/fbt"

