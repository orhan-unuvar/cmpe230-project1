#!/bin/bash
# set -euo pipefail
# IFS=$'\n\t'

cd $(dirname $0)

PROGRAM=../../matlang2c


run_fail_test() {
    # echo "Running fail test $1"
    output=$($PROGRAM $1)
    if [[ $? -ne 1 ]]; then
        echo "Fail test $1 failed. Expected compilation to fail with error code 1."
        exit 1
    fi
    expected=$(cat ${1/%mat/out})
    if [[ $output != $expected ]]; then
        echo "Fail test $1 failed. Expected output: "
        echo "$expected"
        echo "Actual output: "
        echo "$output"
        exit 1
    fi
    echo "Fail test $1 passed."
}

echo "Starting failure tests..."
for i in $(ls -1 ./*.mat); do
    run_fail_test $i
done
echo "Failure tests complete!"

echo "All tests successfully passed!"