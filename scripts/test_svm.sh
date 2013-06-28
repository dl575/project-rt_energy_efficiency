#!/bin/bash -x

###################################################################
# test_svm.sh data_file
###################################################################

for i in {30000..50000..1000}
do
  echo "Threshold of $i us"

  # ../scripts/process_ist.py ../data/atom_dm2_ist $i > out_dm2
  # ../scripts/process_ist.py ../data/atom_mu_ist $i > out_mu

  # svm-train out_dm2
  # svm-train out_mu

  # svm-predict out_dm2 out_mu.model results
  # svm-predict out_mu out_dm2.model results

  # Create train and test set
  ../scripts/process_ffplay.py $1 ist $i
  ../scripts/svm_self.sh ist
  # # First 100 lines for training
  # head -n 100 ist > train
  # # Everything else for testing
  # tail -n +101 ist > test

  # # Train SVM
  # svm-train train
  # # Test SVM
  # svm-predict test train.model results

  # # Cleanup
  # #rm train train.model test results ist
done
