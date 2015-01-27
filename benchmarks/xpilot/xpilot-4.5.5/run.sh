#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency
BENCHMARK=xpilot_slice
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

#if [[ $2 && $2 == "prediction" ]] ; then
if [[ $2 ]] ; then
    echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
    sleep 1;
    echo $2"..."
    taskset $TASKSET_FLAG ./src/server/xpilots > $2 &
    sleep 3;
    taskset $TASKSET_FLAG ./src/client/xpilot &
    
    #find the window 
    xdotool search --sync --onlyvisible --class "xpilot"  
    #maximize the window
    xdotool key alt+F10
    #press join
    xdotool mousemove 65 95
    xdotool click 1
    sleep 3
    #press click
    xdotool mousemove 490 100
    xdotool click 1
    sleep 3
    #playing
    for j in {1..10}
    do
        xdotool keydown Return 
        xdotool keydown shift+a
        sleep 15
        xdotool keyup Return 
        xdotool keyup shift+a
    done
    sleep 3
    #press QUIT
    xdotool mousemove 40 300
    xdotool click 1
    sleep 3
    
    PID_FREECIV_SERVER=$(pgrep 'xpilot')
    kill -9 $PID_FREECIV_SERVER
    mv $2 $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK/$2
    
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
    taskset $TASKSET_FLAG ./src/server/xpilots > $i &
    taskset $TASKSET_FLAG ./src/client/xpilot &
  
    #find the window 
    xdotool search --sync --onlyvisible --class "xpilot"  
    #maximize the window
    xdotool key alt+F10
    #press join
    xdotool mousemove 65 95
    xdotool click 1
    sleep 3
    #press click
    xdotool mousemove 490 100
    xdotool click 1
    sleep 3
    #playing
    for j in {1..10}
    do
        xdotool keydown Return 
        xdotool keydown shift+a
        sleep 15
        xdotool keyup Return 
        xdotool keyup shift+a
    done
    sleep 3
    #press QUIT
    xdotool mousemove 40 300
    xdotool click 1
    sleep 3
    
    PID_FREECIV_SERVER=$(pgrep 'xpilot')
    kill -9 $PID_FREECIV_SERVER
    mv $i $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK/$i
done

#SET TO PERFORMANCE AFTER RUN ALL
echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 

echo "[ done ]"
exit 1
