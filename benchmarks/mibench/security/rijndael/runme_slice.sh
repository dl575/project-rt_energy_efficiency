#!/bin/sh
#./rijndael_slice input_large.asc output_large.enc e 1234567890abcdeffedcba09876543211234567890abcdeffedcba0987654321
for i in {1..200}
do
  key=$(cat /dev/urandom | LC_CTYPE=C tr -dc 'a-f0-9' | fold -w 64 | head -n 1)
  ./input_generator.py > input_slice.asc
  ./rijndael input_slice.asc output_large.enc e $key
done

#./rijndael_slice output_large.enc output_large.dec d 1234567890abcdeffedcba09876543211234567890abcdeffedcba0987654321

