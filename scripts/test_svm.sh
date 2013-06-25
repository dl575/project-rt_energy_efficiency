#!/bin/bash -x

../scripts/process_ist.py out_dm2_raw $1 > out_dm2
../scripts/process_ist.py out_mu_raw $1 > out_mu

svm-train out_dm2
svm-train out_mu

svm-predict out_dm2 out_mu.model results
svm-predict out_mu out_dm2.model results
