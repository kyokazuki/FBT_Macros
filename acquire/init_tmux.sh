# Initiate aquisition environment on DAQ
# share public key beforehand
# ssh-copy-id daq@192.168.253.231
# share NFS server
# sudo exportfs
# mount on xdaq
# sudo mound -t nfs4 192.168.253.214:/mnt/daq_data/daq_shared /mnt/daq_shared
# Si IP
# trb@192.168.253.205:/home/trb

### Config ###
SESSION=daq
BUILD_DIR=~/sw_daq_tofpet2-2024.08.12/build
VME_LOGIN="daq@192.168.253.231"
##############

cd ${BUILD_DIR}

tmux new-session -d -s $SESSION

tmux split-window -h -t $SESSION
tmux split-window -v -t $SESSION:0.0

tmux send-keys -t $SESSION:0.1 './daqd  --socket-name=/tmp/d.sock --daq-type=GBE' C-m
tmux send-keys -t $SESSION:0.2 'ssh -o BatchMode=yes '${VME_LOGIN} C-m

tmux select-pane -t $SESSION:0.0
tmux resize-pane -D 10

tmux a -t $SESSION
