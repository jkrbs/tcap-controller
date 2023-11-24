#!/bin/sh
make

sudo bash <<EOF
export SDE_INSTALL=/home/jakob/bf-sde-9.13.0/install
export LD_LIBRARY_PATH=/usr/local/lib:$SDE_INSTALL/lib:$LD_LIBRARY_PATH
export GLOG_minloglevel=0
export GLOG_logtostderr=1
./control-plane --config=cap-manager --interface=veth9 --address=10.0.0.9 --port=1234
EOF