#!/bin/bash

###################################################
# usage: svm_self.sh libsvmfile 
###################################################

svm-scale $1 > scale
# First 500 lines for training
head -n 1000 scale > train
# Everything else for testing
tail -n +1001 scale > test
#tail -n +1001 scale | head -n 100 > test

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

