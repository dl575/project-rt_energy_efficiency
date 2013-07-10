#!/bin/bash

###################################################
# usage: svm_self.sh libsvmfile 
###################################################

# Chop off first 500 frames, maybe noise or initialization issues
#tail -n +501 $1 > short
svm-scale $1 > scale
#cat $1 > scale
# First 100 lines for training
head -n 500 scale > train
# Everything else for testing
tail -n +501 scale > test
#tail -n +101 scale | head -n 100 > test

# Baseline guessing
../scripts/slow_fast.py test
# Train SVM
svm-train train
# Test SVM
svm-predict test train.model results

# Find per category accuracy
../scripts/compare_svm_results.py test results

# Cleanup
rm train train.model test scale 

