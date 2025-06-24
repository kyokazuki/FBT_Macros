#!/bin/bash

SESSION=daq
BUILD_DIR=~/sw_daq_tofpet2-2024.08.12/build
VME_IP=10.32.19.169

cd ${BUILD_DIR}

tmux new-session -d -s $SESSION

tmux split-window -h -t $SESSION
tmux split-window -v -t $SESSION:0.0

tmux send-keys -t $SESSION:0.1 './daqd  --socket-name=/tmp/d.sock --daq-type=GBE' C-m
tmux send-keys -t $SESSION:0.2 'ssh -o BatchMode=yes daq@'${VME_IP} C-m
tmux send-keys -t $SESSION:0.2 'babicon' C-m

tmux select-pane -t $SESSION:0.0
tmux resize-pane -D 10

tmux a -t $SESSION
