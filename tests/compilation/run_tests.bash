#!/bin/bash
# set -euo pipefail
# IFS=$'\n\t'

cd $(dirname $0)

PROGRAM=../../matlang2c

run_test() {
    echo "Running test $1"
    output=$($PROGRAM $1)
    if [[ $? -ne 0 ]]; then
        echo "Test $1 failed. Could not compile with matlang2c."
        exit 1
    fi
    gcc -ggdb ${1/mat/c} -lm -o prog
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

run_fail_test() {
    echo "Running fail test $1"
    output=$($PROGRAM $1)
    if [[ $? -ne 1 ]]; then
        echo "Fail test $1 failed. Expected compilation to fail with error code 1."
        exit 1
    fi
    expected=$(cat ${1/mat/out})
    if [[ $program_out != $expected ]]; then
        echo "Fail test $1 failed. Expected output: "
        echo "$expected"
        echo "Actual output: "
        echo "$program_out"
        exit 1
    fi
    echo "Fail test $1 passed."
}

cp ../../preamble.h .

run_test ./test1.mat
run_test ./fibonacci.mat
run_test ./choose.mat
run_test ./page_rank.mat
run_test ./stack_test.mat

rm prog
