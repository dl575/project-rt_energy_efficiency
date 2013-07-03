#!/bin/bash
rm output_sweep.txt
for i in {1..2000}
do
  bitcnts $[$RANDOM * 10] >> output_sweep.txt
done
