#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency
BENCHMARK=julius_slice
GOVERNORS=( "performance" "interactive" "conservative" "ondemand" "powersave" ) 

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

sudo chmod 777 /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
sudo chmod 777 /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq
sudo chmod 777 /sys/bus/i2c/drivers/INA231/$SENSOR_ID/sensor_W
sudo chmod 777 /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_cur_freq

echo $BENCHMARK">>>"

if [[ $2 ]] ; then
    if [[ $3 ]] ; then
        MAX_FREQ=$3
    fi
    echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
    sleep 1;
    echo $2"... (freq = "$MAX_FREQ")"
    rm -rf times.txt
    taskset $TASKSET_FLAG ./generate_wav_odroid.py
    mv times.txt $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK/$2
    echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
    echo [ done ]
    exit 1
fi

for i in "${GOVERNORS[@]}"
do 
    echo $i > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
    sleep 1;
    echo $i"..."
    rm -rf times.txt
    taskset $TASKSET_FLAG ./generate_wav_odroid.py
    mv times.txt $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK/$i
done

#SET TO PERFORMANCE AFTER RUN ALL
echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 

echo "[ done ]"
exit 1
