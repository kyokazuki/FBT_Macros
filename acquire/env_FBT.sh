#!/bin/sh

# daq variables
BUILD_DIR=/home/daq/sw_daq_tofpet2-2025.08.04/build # software directory
DAQ_DIR=/home/daq/daq_setup10/test_cable  # config directory
DATA_DIR=tot_adjust	# data directory (appended to DAQ_DIR)
RUN_NUM=0001
DATA_NAME=bg # data file name appended to run number
OV=2.8
TH=20
TIME=86400 # acquisition time in seconds (86400: 1 day, 604800: 1 week)
EXT=1 # external gating enabled if 1

