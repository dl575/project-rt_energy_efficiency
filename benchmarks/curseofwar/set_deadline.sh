#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

DATA_ODROID_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/data_odroid/
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/
SOURCE_FILES=("curseofwar/main.c")
SOURCE_PATH=("curseofwar") 

PREDICT_ENABLED="PREDICT_EN 1"
PREDICT_DISABLED="PREDICT_EN 0"

OVERHEAD_ENABLED="OVERHEAD_EN 1"
OVERHEAD_DISABLED="OVERHEAD_EN 0"

GET_PREDICT_ENABLED="GET_PREDICT 1"
GET_PREDICT_DISABLED="GET_PREDICT 0"

GET_OVERHEAD_ENABLED="GET_OVERHEAD 1"
GET_OVERHEAD_DISABLED="GET_OVERHEAD 0"

DELAY_ENABLED="DELAY_EN 1"
DELAY_DISABLED="DELAY_EN 0"
# big
WHICH_CPU="cpu4"
TASKSET_FLAG="0xf0"
MAX_FREQ=2000000
SENSOR_ID="3-0040"

sudo chmod 777 /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
sudo chmod 777 /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq
sudo chmod 777 /sys/bus/i2c/drivers/INA231/$SENSOR_ID/sensor_W
sudo chmod 777 /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_cur_freq

# ALL disable, run performance
sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$GET_OVERHEAD_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
find . -type f | xargs -n 5 touch
taskset 0xff make clean
taskset 0xff make -j16

echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
sleep 3

rm -rf times.txt
taskset $TASKSET_FLAG ./curseofwar
mv times.txt $BENCH_PATH/$SOURCE_PATH/M0.txt

# prediction/get_predict enable, others disable, run prediction
sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$GET_OVERHEAD_DISABLED"'/'"$GET_OVERHEAD_ENABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
find . -type f | xargs -n 5 touch
taskset 0xff make clean
taskset 0xff make -j16

echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
sleep 3

rm -rf times.txt
taskset $TASKSET_FLAG ./curseofwar
mv times.txt $BENCH_PATH/$SOURCE_PATH/M1M2.txt

echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
sleep 3

taskset 0xff $DATA_ODROID_PATH/find_deadline.py M0.txt M1M2.txt

exit 0





