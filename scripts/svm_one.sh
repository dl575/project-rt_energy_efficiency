#!/bin/bash

###################################################
# usage: svm_one.sh libsvmfile [numtrain]
###################################################

# Parse argument for number of samples for training
if [ -z "$3" ]
then
  numtrain=256
else
  numtrain=$3
fi

# Create training and test sets
svm-subset -s 0 $1 $numtrain train test
# Use full original set for testing in order to calculate percentiles (FIXME)
cp $1 test
echo "Training set:"
svm_compare_results.py train train
#svm_compare_results2.py train train

# Run SVM
svm-easy train test

# Baseline guessing
echo "Testing set:"
svm_slow_fast.py test
# Find per category accuracy
svm_compare_results.py test test.predict
#svm_compare_results2.py test test.predict

percentile.py $2

# Cleanup
#rm train* test*

