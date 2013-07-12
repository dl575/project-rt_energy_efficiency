#!/bin/bash

if [ -z "$1" ]
then
  echo "No input file specified"
  echo "usage: svm_sweep_threshold.py inputtrace"
  exit
fi

SVMFILE=$1.svm
OUTFILE=svm_out

#PARSE_SCRIPT=ffmpeg_parse_metrics_full.py
PARSE_SCRIPT=ffmpeg_parse_metrics.py

rm $OUTFILE
rm $SVMFILE

# Parse to find average
$PARSE_SCRIPT $1 $SVMFILE > parse_out
average=`grep Average parse_out` 
if [[ $average =~ "Average frame time = "([0-9]+) ]]
then
  average=${BASH_REMATCH[1]}
fi
# Step size is 5% of average
step=$[$average / 20]
rm parse_out

for ((i = $[$average - 4*$step]; i <= $[$average + 4*step]; i += $step))
do
  echo "Threshold of $i us" | tee --append $OUTFILE

  # Parse metrics into libsvm format from raw trace file
  $PARSE_SCRIPT $1 $SVMFILE $i
  # Perform SVM classification with train and test from one data set
  svm_one.sh $SVMFILE >> $OUTFILE
done
