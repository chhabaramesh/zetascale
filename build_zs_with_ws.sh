#!/bin/bash

ls -l libws.so
export LIBRARY_PATH=`pwd`
echo "Lib path set to $LIBRARY_PATH"

./build_zs_sdk.sh



