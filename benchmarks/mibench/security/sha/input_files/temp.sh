#!/bin/bash
for i in {1..100}
do
  ../input_generator.py > input_random"$i".asc
done
