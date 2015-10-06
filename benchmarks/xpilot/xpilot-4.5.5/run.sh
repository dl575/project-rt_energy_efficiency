#!/bin/bash

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency
BENCHMARK_FOLDER=xpilot_slice
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
init big
init little

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
    taskset $TASKSET_FLAG ./src/server/xpilots > $2 &
    sleep 3;
    taskset $TASKSET_FLAG ./src/client/xpilot &
    
    PROCESS_CNT_BEFORE=$(pgrep -c 'xpilot')
    #find the window 
    xdotool search --sync --onlyvisible --class "xpilot"  
    #maximize the window
    xdotool key alt+F10
    sleep 3
    #press join
    xdotool mousemove 65 95
    sleep 3
    xdotool click 1
    sleep 3
    #press click
    xdotool mousemove 490 100
    sleep 3
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
    
    PID_FREECIV_SERVER=$(pgrep 'xpilot')
    kill -9 $PID_FREECIV_SERVER
 
    cp $2 $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK_FOLDER/$BENCHMARK/$2
else
    echo "specify governor!"
    exit 1
fi

#SET TO PERFORMANCE AFTER RUN ALL
init big
init little

echo "[ run.sh "$2" done ]"
exit 0

