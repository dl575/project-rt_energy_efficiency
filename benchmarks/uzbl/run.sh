#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency
BENCHMARK=uzbl
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
    echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
    sleep 1;
    echo $2"..."
    taskset $TASKSET_FLAG uzbl-browser > output_slice.txt &
    sleep 10;

    #find the window 1
    xdotool search --sync --onlyvisible --class "uzbl"  
    #maximize the window 1
    xdotool key alt+F10;            sleep 1;
    #type url, go to csl site
    xdotool key o;                  sleep 1;
    xdotool type csl.cornell.edu;   sleep 1;
    xdotool key Return;             sleep 10;
    #move mouse and click
    #xdotool getmouselocation --shell
    #click people tab
    xdotool mousemove 268 342;
    xdotool click 1                 sleep 10;
    #click research tab
    xdotool mousemove 475 341;
    xdotool click 1                 sleep 10;
    #refresh
    xdotool key r;                  sleep 10;
    #back
    xdotool key b;                  sleep 10;
    #forward
    xdotool key m;                  sleep 10;
    #scroll
    xdotool key j;                  sleep 1;
    xdotool key j;                  sleep 1;
    xdotool key j;                  sleep 1
    xdotool key k;                  sleep 1;
    xdotool key k;                  sleep 1;
    xdotool key k;                  sleep 1;
    xdotool key l;                  sleep 1;
    xdotool key l;                  sleep 1;
    xdotool key l;                  sleep 1;
    xdotool key h;                  sleep 1;
    xdotool key h;                  sleep 1;
    xdotool key h;                  sleep 1;
    #close 1
    xdotool key alt+F4;             sleep 1;
    
    mv output_slice.txt $PROJECT_PATH/dvfs_sim/data_odroid/$SAVED_FOLDER/$BENCHMARK/$2
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
    taskset $TASKSET_FLAG uzbl-browser > output_slice.txt
    taskset $TASKSET_FLAG ./runme_slice.sh
    mv output_slice.txt $PROJECT_PATH/dvfs_sim/data_odroid/$SAVED_FOLDER/$BENCHMARK/$i
done

#SET TO PERFORMANCE AFTER RUN ALL
echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 

echo "[ done ]"
exit 1
