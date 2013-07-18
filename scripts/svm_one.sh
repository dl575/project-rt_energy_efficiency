#!/bin/bash

###################################################
# usage: svm_one.sh libsvmfile [numtrain]
###################################################

# Parse argument for number of samples for training
if [ -z "$2" ]
then
  numtrain=100
else
  numtrain=$2
fi

# Create training and test sets
svm-subset -s 0 $1 $numtrain train test

# Run SVM
svm-easy train test

# Baseline guessing
svm_slow_fast.py test
# Find per category accuracy
svm_compare_results.py test test.predict

# Cleanup
#rm train* test*

