#!/bin/sh

# daq variables
BUILD_DIR=/home/daq/sw_daq_tofpet2-2026.03.20/build_disc # software directory
DAQ_DIR=/home/daq/daq_setup11/FDC1_test # config directory
# DATA_DIR=fdc1_noise # data directory (appended to DAQ_DIR)
DATA_DIR=daq_debug # data directory (appended to DAQ_DIR)
DATA_NAME=pulse # data file name appended to run number
OV=2.8 # overvoltage
TH=20 # threshold
TIME=86400 # acquisition time in seconds (86400: 1 day, 604800: 1 week)
#TIME=30 # acquisition time in seconds (86400: 1 day, 604800: 1 week)
EXT=1 # external gate enabled if 1
RENAME=0 # rename time to live time if 1
CONVERT=1 # convert to root if 1
COPY=1 # copy data if 1
COPY_KEY="/home/daq/.ssh/id_ed25519_sana01" # ssh key to use
COPY_DEST="s074@sana01.ribfdaq:/home/s074/fbt/FDC1_test/fdc1" # data copy destination

# dt_range is {-580000, -540000}
# /home/s074/exp/exp2605_s074/anaroot/users/hanai
# (xiX-2.5-160)*0.75

