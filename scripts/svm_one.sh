#!/bin/bash

###################################################
# usage: svm_one.sh libsvmfile 
###################################################

# Chop off first 500 frames, maybe noise or initialization issues
#tail -n +501 $1 > short
#svm-scale $1 > scale
#cat $1 > scale

# First 100 lines for training
head -n 500 $1 > train
# Everything else for testing
tail -n +501 $1 > test

# # Train SVM
# svm-train train
# # Test SVM
# svm-predict test train.model results

# Run SVM
svm-easy train test

# Baseline guessing
svm_slow_fast.py test
# Find per category accuracy
svm_compare_results.py test test.predict

# Cleanup
#rm train* test*

