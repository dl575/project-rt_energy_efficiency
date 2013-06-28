#!/bin/bash

###################################################
# usage: svm_self.sh libsvmfile 
###################################################

svm-scale $1 > scale
# First 500 lines for training
head -n 100 scale > train
# Everything else for testing
tail -n +101 scale | head -n 100 > test

# Train SVM
svm-train train
# Test SVM
svm-predict test train.model results

# Cleanup
rm train train.model test results scale

