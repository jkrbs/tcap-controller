#!/bin/bash
if [ "$1" == "deploy" ]; then
       rsync -avr . tofino1:jakob/cap-mgr-cp/
       exit
fi

make

sudo bash <<EOF
export SDE=/home/ubuntu/bf-sde-9.11.0
export SDE_INSTALL=$SDE/install
export LD_LIBRARY_PATH=/usr/local/lib:$SDE_INSTALL/lib:$LD_LIBRARY_PATH
export GLOG_minloglevel=0
export GLOG_logtostderr=1
./control-plane --switch-config=cap-manager --interface=veth9 --address=10.0.9.2 --port=1234 --config=sim.json
EOF