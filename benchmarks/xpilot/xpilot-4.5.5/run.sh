#!/bin/bash

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency
BENCHMARK_FOLDER=xpilot_slice
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

PID_FREECIV_SERVER=$(pgrep 'xpilot')
kill -9 $PID_FREECIV_SERVER

echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 

echo $BENCHMARK">>>"
echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor

if [[ $2 ]] ; then
    mkdir -p $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK_FOLDER/$BENCHMARK
    echo $2 > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    if [[ $4 ]] ; then
        echo $4 > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
    fi
    sleep 1;
    echo $2"..."
    taskset $TASKSET_FLAG ./src/server/xpilots > $2 &
    sleep 3;
    taskset $TASKSET_FLAG ./src/client/xpilot &
    
    PROCESS_CNT_BEFORE=$(pgrep -c 'xpilot')
    #find the window 
    xdotool search --sync --onlyvisible --class "xpilot"  
    #maximize the window
    xdotool key alt+F10
    #press join
    xdotool mousemove 65 95
    sleep 1
    xdotool click 1
    sleep 3
    #press click
    xdotool mousemove 490 100
    sleep 1
    xdotool click 1
    sleep 3
    #playing
    while true;
    do
        if [[ $PROCESS_CNT_BEFORE > $(pgrep -c 'xpilot') ]] ; then
            echo "xpilot ends"
            break
        fi
        xdotool keydown Return 
        xdotool keydown shift+a
        sleep 3
        xdotool keyup Return 
        xdotool keyup shift+a
    done
    echo "xdotool done"
    #press QUIT
    #xdotool mousemove 40 300
    #xdotool click 1
    #sleep 3
    
#    PID_FREECIV_SERVER=$(pgrep 'xpilot')
#    kill -9 $PID_FREECIV_SERVER
 
    cp $2 $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK_FOLDER/$BENCHMARK/$2
else
    echo "specify governor!"
    exit 1
fi

#SET TO PERFORMANCE AFTER RUN ALL
echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 

echo "[ run.sh "$2" done ]"
exit 0
