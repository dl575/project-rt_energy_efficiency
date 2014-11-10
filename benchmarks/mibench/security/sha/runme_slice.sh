#!/bin/sh
rm output_slice.txt
# ./sha input_small.asc > output_slice.txt
# ./sha input_large.asc >> output_slice.txt
for i in {1..100}
do
  ./input_generator.py > input_random.asc
  ./sha input_random.asc >> output_slice.txt
done
