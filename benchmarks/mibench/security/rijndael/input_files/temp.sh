#!/bin/bash
#./rijndael_slice input_large.asc output_large.enc e 1234567890abcdeffedcba09876543211234567890abcdeffedcba0987654321
for i in {1..100}
do
  ../input_generator.py > input_random"$i".asc
done

#./rijndael_slice output_large.enc output_large.dec d 1234567890abcdeffedcba09876543211234567890abcdeffedcba0987654321

