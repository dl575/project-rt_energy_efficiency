#!/bin/bash -x

for i in {10000..70000..5000}
do
  echo "Threshold of $i us"

  # ../scripts/process_ist.py ../data/atom_dm2_ist $i > out_dm2
  # ../scripts/process_ist.py ../data/atom_mu_ist $i > out_mu

  # svm-train out_dm2
  # svm-train out_mu

  # svm-predict out_dm2 out_mu.model results
  # svm-predict out_mu out_dm2.model results

  # Create train and test set
  ../scripts/process_ist.py ../data/atom_mu_ist $i > ist
  head -n 100 ist > train
  tail -n +101 ist > test

  svm-train train
  svm-predict test train.model results

  # Cleanup
  rm train train.model test results ist
done
