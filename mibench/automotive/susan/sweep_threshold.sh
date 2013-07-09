
export RUNDIR=run
OUTFILE=$RUNDIR/svm_out

rm $OUTFILE

for i in {100000..400000..25000}
do
  echo "Threshold of $i" | tee --append $OUTFILE
  ./svm_one.sh $i | tee --append $OUTFILE
done
