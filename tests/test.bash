#!/bin/bash
set -euo pipefail

cd $(dirname $0)

./test
bash compilation/run_tests.bash
bash fails/run_tests.bash