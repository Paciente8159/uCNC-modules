#!/usr/bin/env bash

make clean
export MAKE_TEST=ALL_AT_ONCE
echo "Running test $MAKE_TEST"
make run

make clean
export MAKE_TEST=ROW_BY_ROW
echo "Running test $MAKE_TEST"
make run

# make clean
# export MAKE_TEST=PIXEL_BY_PIXEL
# echo "Running test $MAKE_TEST"
# make run

