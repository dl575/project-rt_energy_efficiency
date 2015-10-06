#!/bin/bash

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency
BENCHMARK_FOLDER=uzbl
BENCHMARK=$BENCHMARK_FOLDER"-"$3

if [[ $# < 3 ]] ; then
    echo 'USAGE : ./run.sh [big/little] [governors] [sweep]'
    exit 1
fi

if [ $1 != "big" ] && [ $1 != "little" ] && [ $1 != "hetero" ] ; then
    echo 'USAGE : ./run.sh [big/little/hetero]'
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
elif [ $1 == "hetero" ] ; then
    WHICH_CPU="cpu0"
    TASKSET_FLAG="0x0f"
    MAX_FREQ=1400000
    SENSOR_ID="3-0045"
fi

init(){
    if [ $1 == "big" ] ; then
        echo big
        sudo chmod 777 /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor
        sudo chmod 777 /sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq
        sudo chmod 777 /sys/bus/i2c/drivers/INA231/3-0040/sensor_W
        sudo chmod 777 /sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq
        echo 2000000 > /sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq 
        echo performance > /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor
    elif [ $1 == "little" ] ; then
        echo little
        sudo chmod 777 /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
        sudo chmod 777 /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
        sudo chmod 777 /sys/bus/i2c/drivers/INA231/3-0045/sensor_W
        sudo chmod 777 /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
        echo 1400000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 
        echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    fi
}

echo $BENCHMARK">>>"
if [ $1 != "hetero" ] ; then
    init $1
else
    init big
    init little
fi

if [[ $2 ]] ; then
    mkdir -p $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK_FOLDER/$BENCHMARK
    echo $2 > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    if [[ $4 ]] ; then
		if [ $3 == "freq_sweep" ] ; then
			echo $4 > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
		elif [ $3 == "freq_other_sweep" ] ; then
    		if [ $1 == "big" ] ; then
				echo 1400000 > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
				echo $4 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 
    		elif [ $1 == "little" ] ; then
				echo 1400000 > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
				echo $4 > /sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq 
			fi
		fi
    fi
    sleep 1;
    echo $2"..."
    taskset $TASKSET_FLAG uzbl-browser > output_slice.txt &
    sleep 60;
    echo "xdotool";

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
    #xdotool mousemove 268 342;
    xdotool mousemove 337 343;
    xdotool click 1                 sleep 10;
    #click research tab
    #xdotool mousemove 475 341;
    xdotool mousemove 536 342;
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
    #    xdotool key alt+F4;             sleep 1;
    #make sure to close by mouse click
    #xdotool mousemove 1134 2;       sleep 1;
    xdotool mousemove 1267 2;       sleep 1;
    xdotool click 1                 sleep 1;
    xdotool mousemove 786 16;       sleep 1;
    xdotool click 1                 sleep 1;
    sleep 30
 
    mv output_slice.txt $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK_FOLDER/$BENCHMARK/$2
e
else
    echo "specify governor!"
    exit 1
fi

#SET TO PERFORMANCE AFTER RUN ALL
if [ $1 != "hetero" ] ; then
    init $1
else
    init big
    init little
fi

echo "[ run.sh "$2" done ]"
exit 0

