#!/bin/bash

echo "interference start"
for (( j=0; j<10000; j++ ));
do
  taskset 0x08 echo "interference"
done
echo "interference end"
