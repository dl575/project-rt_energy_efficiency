#!/bin/bash

if [ ! -f susan ]
then
  echo "susan not build. Run make first."
  exit
fi

RUNDIR=run
OUTFILE=$RUNDIR/out

mkdir $RUNDIR
rm $OUTFILE

for i in {1..250}
do
  W=$[RANDOM % 1024 + 100]
  H=$[RANDOM % 1024 + 100]
  echo "Iteration $i, Size = ($W, $H)" | tee -a $OUTFILE
  pgmnoise $W $H > $RUNDIR/input_random.pgm

  ./susan $RUNDIR/input_random.pgm $RUNDIR/output.pgm -s >> $OUTFILE
  ./susan $RUNDIR/input_random.pgm $RUNDIR/output.pgm -e >> $OUTFILE
  ./susan $RUNDIR/input_random.pgm $RUNDIR/output.pgm -c >> $OUTFILE
done

