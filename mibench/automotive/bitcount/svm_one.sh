
cd $RUNDIR

# Parse into svm file with thresholding applied
../parse.py output_sweep.txt svm $1
# Split into train and test sets
head -n 200 svm > train
tail -n +201 svm > test
# Run SVM
svm-easy train test
# Find guessing results
slow_fast.py test
# Find percentage of slow/fast correct
compare_svm_results.py test test.predict

# Clean up
rm test* train*

cd ..

