#!/usr/bin/env bash
TIMEFORMAT=%R

rm -rf tests
mkdir tests
touch ./tests/runtime

for run in {1..100}
do
    echo ${run} | tee -a ./tests/runtime 1> /dev/null
    { time  { ./bmatching ${run} input.txt 20 1> ./tests/${run}.out; } ; }  2>&1 | tee -a ./tests/runtime  > /dev/null
    echo "/n"  1>| tee -a ./tests/runtime
done
