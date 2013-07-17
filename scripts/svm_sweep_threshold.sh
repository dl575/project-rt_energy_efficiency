#!/bin/bash

if [ -z "$1" ]
then
  echo "No input file specified"
  echo "usage: svm_sweep_threshold.py inputtrace"
  exit
fi

SVMFILE=$1.svm

echo "Select parse script to use:"
echo "  [1] ffmpeg_parse_metrics.py"
echo "  [2] ffmpeg_parse_metrics_full.py"
echo "  [3] ffmpeg_parse_metrics_slice.py"
echo "  [4] ffmpeg_parse_metrics_slice_full.py"
read parse_option

if [ -z $parse_option ] 
then
  echo "No option specified."
  exit
elif [ $parse_option == 1 ]
then
  PARSE_SCRIPT=ffmpeg_parse_metrics.py
elif [ $parse_option == 2 ]
then
  PARSE_SCRIPT=ffmpeg_parse_metrics_full.py
elif [ $parse_option == 3 ]
then
  PARSE_SCRIPT=ffmpeg_parse_metrics_slice.py
elif [ $parse_option == 4 ]
then
  PARSE_SCRIPT=ffmpeg_parse_metrics_slice_full.py
else
  echo "Error: Unrecognized parse script option."
  exit
fi

OUTFILE=svm_out.parse$parse_option

rm $OUTFILE
rm $SVMFILE

# Parse to find average
$PARSE_SCRIPT $1 $SVMFILE > parse_out
average=`grep Average parse_out` 
if [[ $average =~ "Average frame time = "([0-9]+) ]]
then
  average=${BASH_REMATCH[1]}
fi

# Step size 
step=$[$average / 5]
# Number of points
numpoints=11
rm parse_out

for ((i = $[$average - ($numpoints - 1)/2*$step]; i <= $[$average + ($numpoints - 1)/2*$step]; i += $step))
do
  echo "Threshold of $i us" | tee --append $OUTFILE

  # Parse metrics into libsvm format from raw trace file
  $PARSE_SCRIPT $1 $SVMFILE $i
  # Perform SVM classification with train and test from one data set
  svm_one.sh $SVMFILE | tee --append $OUTFILE
done
