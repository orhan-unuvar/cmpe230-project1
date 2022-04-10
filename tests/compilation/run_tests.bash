#!/bin/bash

cd $(dirname $0)

PROGRAM=../../matlang2c

run_test() {
    # echo "Running test $1"
    output=$($PROGRAM $1)
    if [[ $? -ne 0 ]]; then
        echo "Test $1 failed. Could not compile with matlang2c."
        echo "Output: $output"
        exit 1
    fi
    gcc ${1/%mat/c} -lm -o prog
    if [[ $? -ne 0 ]]; then
        echo "Test $1 failed. Could not compile generated C code."
        exit 1
    fi
    program_out=$(./prog)
    expected=$(cat ${1/%mat/out})
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

echo "Starting compilation tests..."
for i in $(ls -1 ./*.mat); do
    run_test $i
done
echo "Compilation tests complete!"

rm prog
