#!/bin/bash
# Disassemble a sanitiser coverage build of coverage_main.cpp. Every step must
# succeed: disassembling a stale or absent test_output would produce a
# "coverage report" for a binary this script did not build.
set -euo pipefail

# clang 4.0 is the release whose -fsanitize-coverage understands all the flags
# below, so it stays the default; CXX lets a newer clang be substituted.
: "${CXX:=clang++-4.0}"

#clang++-4.0 -std=c++1z -shared -fPIC -fvisibility=hidden -o test_output.so coverage_main.cpp -I.. -fsanitize-coverage=trace-pc,indirect-calls -fsanitize-coverage=trace-cmp -fsanitize-coverage=trace-div -fsanitize-coverage=trace-gep
"$CXX" -std=c++1z -O3 -g -o test_output coverage_main.cpp -I.. -fsanitize-coverage=trace-pc,indirect-calls -fsanitize-coverage=trace-cmp -fsanitize-coverage=trace-div -fsanitize-coverage=trace-gep
objdump -d test_output > test_output.s
