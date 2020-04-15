#!/bin/bash

# test runner program over many runs to see concurrency problems /
# repeated runs of jobs.

if [[ "$1" == "" ]]; then
    printf "usage: ./test_runner {runner_nosem, runner_sem1, runner_sem2}\n"
    exit 1
fi

runner=$1

jobsfile=jobs.txt.2
rm -f x.out
./$runner -init

for r in $(seq 25); do
    echo "ITER $r";
    cp $jobsfile jobs.txt       # copy the jobs file back
    for i in $(seq 10); do
        ./$runner jobs.txt &
    done | tee x.out | sort | uniq -w 3 -D # sort output and look for duplicat job runs
done
