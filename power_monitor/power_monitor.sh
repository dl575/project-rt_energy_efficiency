#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

if [[ $# < 1 ]] ; then
    echo 'USAGE : ./run.sh big or ./run.sh little'
    exit 1
fi

if [ $1 != "big" -a $1 != "little" ] ; then
    echo 'USAGE : only big or little'
    exit 1
fi

if [ $1 == "big" ] ; then
    SAVED_FOLDER="big"
    WHICH_CPU="cpu4"
    TASKSET_FLAG="0xf0"
    MAX_FREQ=2000000
    SENSOR_ID="3-0040"
elif [ $1 == "little" ] ; then
    SAVED_FOLDER="little"
    WHICH_CPU="cpu0"
    TASKSET_FLAG="0x0f"
    MAX_FREQ=1400000
    SENSOR_ID="3-0045"
fi

CORE_BIG="CORE 1"
CORE_LITTLE="CORE 0"

if [ $1 == "big" ] ; then
    sed -i -e 's/'"$CORE_LITTLE"'/'"$CORE_BIG"'/g' power_monitor.c
elif [ $1 == "little" ] ; then
    sed -i -e 's/'"$CORE_BIG"'/'"$CORE_LITTLE"'/g' power_monitor.c
fi
find . -type f | xargs -n 5 touch
taskset 0xff make clean
taskset 0xff make -j16

rm -rf output_power.txt

if [ $1 == "big" ] ; then
sudo taskset 0x0f ./power_monitor > output_power.txt
elif [ $1 == "little" ] ; then
sudo taskset 0xf0 ./power_monitor > output_power.txt
fi

