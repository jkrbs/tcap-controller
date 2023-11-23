#!/bin/sh

export LD_LIBRARY_PATH=/usr/local/lib:$SDE_INSTALL/lib:$LD_LIBRARY_PATH
make
./control-plane --config=cap-manager --interface=veth10 --address=10.0.0.1 --port=1234