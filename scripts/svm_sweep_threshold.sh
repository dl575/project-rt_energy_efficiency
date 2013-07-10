#!/bin/bash -x

###################################################################
# test_svm.sh data_file
###################################################################

if [ -z "$1" ]
then
  echo "No input file specified"
  exit
fi

for i in {4000..7000..100}
#for i in {10000..70000..5000}
do
  echo "Threshold of $i us"

  # ../scripts/process_ist.py ../data/atom_dm2_ist $i > out_dm2
  # ../scripts/process_ist.py ../data/atom_mu_ist $i > out_mu

  # svm-train out_dm2
  # svm-train out_mu

  # svm-predict out_dm2 out_mu.model results
  # svm-predict out_mu out_dm2.model results

  # Create train and test set
  #../scripts/process_ist.py $1 ist $i
  ../scripts/process_ffplay.py $1 ist $i
  # Attempts to shift information to frame
  #../scripts/process_ffmpeg.py $1 ist $i
  ../scripts/svm_one.sh ist
done
