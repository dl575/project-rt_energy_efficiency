#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

DATA_ODROID_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/data_odroid/
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/
SOURCE_FILES=("uzbl/src/commands.c")
SOURCE_PATH=("uzbl") 

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
taskset 0xff ./fix_addresses.py 
taskset 0xff make -j16 
taskset 0xff sudo make install 

echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
sleep 3

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
mv output_slice.txt $BENCH_PATH/$SOURCE_PATH/M0.txt

# prediction/get_predict enable, others disable, run prediction
sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$GET_OVERHEAD_DISABLED"'/'"$GET_OVERHEAD_ENABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$SOURCE_FILES
find . -type f | xargs -n 5 touch
taskset 0xff make clean
taskset 0xff make -j16 
taskset 0xff ./fix_addresses.py 
taskset 0xff make -j16 
taskset 0xff sudo make install 

echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
sleep 3

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
mv output_slice.txt $BENCH_PATH/$SOURCE_PATH/M1M2.txt

echo performance > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_governor
echo $MAX_FREQ > /sys/devices/system/cpu/$WHICH_CPU/cpufreq/scaling_max_freq 
sleep 3

taskset 0xff $DATA_ODROID_PATH/find_deadline.py M0.txt M1M2.txt

exit 0





