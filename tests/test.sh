#!/bin/sh

set -e # Fail on any command error

exec_name=$1
filename=$(basename $exec_name).c

../cc $filename -o=$exec_name
my_out=$(lli "${exec_name}.ll")

# Assuming this has been compiled by cmake
cc_out=$($exec_name)

test "$my_out" == "$cc_out"