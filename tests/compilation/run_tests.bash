#!/bin/bash
# set -euo pipefail
# IFS=$'\n\t'

cd $(dirname $0)

PROGRAM=../../matlang2c

run_test() {
    echo "Running test $1"
    exit_code=$($PROGRAM $1)
    if [[ $? -ne 0 ]]; then
        echo "Test $1 failed. Could not compile with matlang2c."
        exit 1
    fi
    gcc -ggdb ${1/mat/c} -o prog
    if [[ $? -ne 0 ]]; then
        echo "Test $1 failed. Could not compile generated C code."
        exit 1
    fi
    program_out=$(./prog)
    expected=$(cat ${1/mat/out})
    if [[ $program_out != $expected ]]; then
        echo "Test $1 failed. Expected output: "
        echo "$expected"
        echo "Actual output: "
        echo "$program_out"
        exit 1
    fi
    echo "Test $1 passed."
}
cp ../../preamble.h .

run_test ./test1.mat
run_test ./fibonacci.mat
run_test ./choose.mat
run_test ./page_rank.mat

rm prog
