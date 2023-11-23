#!/bin/sh

export LD_LIBRARY_PATH=/usr/local/lib:$SDE_INSTALL/lib:$LD_LIBRARY_PATH

./controller -p cap-manager