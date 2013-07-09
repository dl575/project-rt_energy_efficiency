#!/bin/bash

mkdir run
export RUNDIR=`pwd`/run

# Check for raw output file
if [ ! -f $RUNDIR/output_sweep.txt ]
then
  echo "output_sweep.txt not found. Run runme_sweep.sh first"
  exit
fi

rm $RUNDIR/svm_out

for i in {0..5000..500}
do
  echo "Threshold of $i" | tee --append $RUNDIR/svm_out
  ./svm_one.sh $i | tee --append $RUNDIR/svm_out
done
