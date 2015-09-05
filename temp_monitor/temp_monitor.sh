#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

find . -type f | xargs -n 5 touch
taskset 0xff make clean
taskset 0xff make -j16

rm -rf output_temp.txt

exit 0
#if [ $1 == "big" ] ; then
#sudo taskset 0x0f ./temp_monitor > output_temp.txt
#elif [ $1 == "little" ] ; then
#sudo taskset 0xf0 ./temp_monitor > output_temp.txt
#fi

