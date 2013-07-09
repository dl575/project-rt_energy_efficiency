#!/bin/bash

RUNDIR=run

mkdir $RUNDIR
rm $RUNDIR/output_sweep.txt

for i in {1..2000}
do
  # Print out to see progress
  if [ $[$i % 10] -eq 0 ]
  then
    echo -en "\r$i/2000"
  fi
  ./bitcnts $[$RANDOM * 10] >> $RUNDIR/output_sweep.txt
done

echo
