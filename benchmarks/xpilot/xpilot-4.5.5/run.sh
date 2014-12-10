#!/bin/bash
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
elif [ $1 == "little" ] ; then
    WHICH_CPU="cpu0"
    TASKSET_FLAG="0x0f"
    MAX_FREQ=1400000
fi

#if [[ $2 && $2 == "prediction" ]] ; then
if [[ $2 ]] ; then
    echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
    echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
    sleep 1;
    echo $2"..."
    taskset $TASKSET_FLAG ./src/server/xpilots > $2 &
    sleep 3;
    taskset $TASKSET_FLAG ./src/client/xpilot
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
    taskset $TASKSET_FLAG ./src/client/xpilot
    PID_FREECIV_SERVER=$(pgrep 'xpilot')
    kill -9 $PID_FREECIV_SERVER
    mv $i $PROJECT_PATH/dvfs_sim/data_odroid/$1/$BENCHMARK/$i
done

#SET TO PERFORMANCE AFTER RUN ALL
echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 

echo "[ done ]"
exit 1
