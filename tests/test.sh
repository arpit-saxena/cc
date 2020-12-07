#!/bin/sh

set -e # Fail on any command error

exec_name=$1
only_compile_check=$2
filename=$(basename $exec_name).c

# parent_path=$( cd "$(dirname "$0")" ; pwd -P )
# cd $parent_path

../../cc $filename -o=$exec_name

if [[ $only_compile_check -ne 0 ]]
then
    my_out=$(lli "${exec_name}.ll")

    # Assuming this has been compiled by cmake
    cc_out=$($exec_name)

    echo "My output: $my_out"
    echo "gcc output: $cc_out"

    test "$my_out" == "$cc_out"
fi