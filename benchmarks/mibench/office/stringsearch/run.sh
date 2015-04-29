#!/bin/bash

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency
BENCHMARK_FOLDER=stringsearch
BENCHMARK=$BENCHMARK_FOLDER"-"$3

if [[ $# < 3 ]] ; then
    echo 'USAGE : ./run.sh [big/little] [governors] [sweep]'
    exit 1
fi

if [ $1 != "big" -a $1 != "little" ] ; then
    echo 'USAGE : only big or little'
    exit 1
fi

if [ $1 == "big" ] ; then
    WHICH_CPU="cpu4"
    TASKSET_FLAG="0xf0"
    MAX_FREQ=2000000
    SENSOR_ID="3-0040"
elif [ $1 == "little" ] ; then
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
    mkdir -p $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK_FOLDER/$BENCHMARK
    #if [[ $3 ]] ; then
    #    MAX_FREQ=$3
    #fi
    if [[ $2 == "prediction" ]] || [[ $2 == "oracle" ]] || [[ $2 == "pid" ]] ; then    
        echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    else
        echo $2 > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    fi
    sleep 1;
    echo $2"..."
    taskset $TASKSET_FLAG ./runme_large.sh
    mv output_large.txt $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK_FOLDER/$BENCHMARK/$2
else
    echo "specify governor!"
    exit 1
fi

#SET TO PERFORMANCE AFTER RUN ALL
echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 

echo "[ run.sh "$2" done ]"
exit 0
