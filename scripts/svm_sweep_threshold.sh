#!/bin/bash

if [ -z "$1" ]
then
  echo "No input file specified"
  echo "usage: svm_sweep_threshold.py inputtrace"
  exit
fi

SVMFILE=$1.svm
OUTFILE=svm_out

rm $OUTFILE
rm $SVMFILE

for i in {14000..22000..1000}
do
  echo "Threshold of $i us" | tee --append $OUTFILE

  # Parse metrics into libsvm format from raw trace file
  ffmpeg_parse_metrics.py $1 $SVMFILE $i
  # Perform SVM classification with train and test from one data set
  svm_one.sh $SVMFILE >> $OUTFILE
done
