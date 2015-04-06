#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

DATA_ODROID_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/data_odroid
POWER_MONITOR_PATH=/home/odroid/project-rt_energy_efficiency/power_monitor
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/

#:27,58s/^/#
#:27,58s/^#/
SOURCE_FILES=(
"2048.c/2048.c"
)
SOURCE_PATH=(
"2048.c"
)
BENCHMARKS=(
"2048.c"
)
BENCH_NAME=(
"2048_slice"
)

#SOURCE_FILES=(
#"mibench/office/stringsearch/pbmsrch_large.c"
#"mibench/security/sha/sha_driver.c"
#"mibench/security/rijndael/aesxam.c"
#"xpilot/xpilot-4.5.5/src/server/server.c"
#"julius/julius-4.3.1/libjulius/src/recogmain.c"
#"2048.c/2048.c"
#"curseofwar/main.c"
#"uzbl/src/commands.c"
#)
#SOURCE_PATH=(
#"mibench/office/stringsearch"
#"mibench/security/sha"
#"mibench/security/rijndael"
#"xpilot/xpilot-4.5.5"
#"julius/julius-4.3.1"
#"2048.c"
#"curseofwar"
#"uzbl"
#)
#BENCHMARKS=(
#"mibench/office/stringsearch"
#"mibench/security/sha"
#"mibench/security/rijndael"
#"xpilot/xpilot-4.5.5"
#"julius/julius-3.5.2-quickstart-linux"
#"2048.c"
#"curseofwar"
#"uzbl"
#)
#BENCH_NAME=(
#"stringsearch"
#"sha"
#"rijndael"
#"xpilot_slice"
#"julius_slice"
#"2048_slice"
#"curseofwar_slice"
#"uzbl"
#)
#
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


#save current time
CURRENT_TIME=`date`
echo $CURRENT_TIME
sleep 5

TEMP_PWD=`pwd`
PID_POWER_MONITOR=$(pgrep 'power_monitor')
sudo kill -9 $PID_POWER_MONITOR

for (( i=0; i<${#BENCH_NAME[@]}; i++ ));
do
    #with overhead
    cd $POWER_MONITOR_PATH
    echo "entered "`pwd`
    ./power_monitor.sh $1 &
    cd $TEMP_PWD
    echo "entered "`pwd`
    
    #MUST RUN as this order. LINUX Governor > Prediction with overhead > Prediction w/o overhead
    #FOR LINUX GOVERNOR, overhead does not matter
    #you can set overhead_en as well.
    taskset 0xff ./buildAll.sh $i $1 predict_dis overhead_dis
    sleep 3
    ./runAll.sh $i $1
    sleep 3
    #prediction with overhead
    taskset 0xff ./buildAll.sh $i $1 predict_en overhead_en
    sleep 3
    ./runAll.sh $i $1 prediction_with_overhead
    sleep 3
    #prediction without overhead
    taskset 0xff ./buildAll.sh $i $1 predict_en overhead_dis
    sleep 3
    ./runAll.sh $i $1 prediction_wo_overhead
    sleep 3

    PID_POWER_MONITOR=$(pgrep 'power_monitor')
    sudo kill -9 $PID_POWER_MONITOR
    #enter password
    xdotool type odroid
    xdotool key KP_Enter
    
    mv $POWER_MONITOR_PATH/output_power.txt $DATA_ODROID_PATH/$SAVED_FOLDER/${BENCH_NAME[$i]}

    #without overhead
#    cd $POWER_MONITOR_PATH
#    echo "entered "`pwd`
#    ./power_monitor.sh &
#    cd $TEMP_PWD
#    echo "entered "`pwd`

#    taskset 0xff ./buildAll.sh $i predict_en overhead_dis
#    sleep 3
#    ./runAll.sh $i big_wo_overhead prediction
#    sleep 3
#    taskset 0xff ./buildAll.sh $i predict_dis overhead_dis
#    sleep 3
#    ./runAll.sh $i big_wo_overhead
#    sleep 3

#    PID_POWER_MONITOR=$(pgrep 'power_monitor')
#    sudo kill -9 $PID_POWER_MONITOR
#    #enter password
#    xdotool type odroid
#    xdotool key KP_Enter

#    mv $POWER_MONITOR_PATH/output_power.txt $DATA_ODROID_PATH/big_wo_overhead/${BENCH_NAME[$i]}
done

#restore current time
echo "resetting date..."
sudo date --set="$CURRENT_TIME"
sudo date --set="Sun Mar 22 15:30:16 EST 2015"

#filter xpilot_slice
cd $DATA_ODROID_PATH
GOVERNOR_FILES=( "performance" "interactive" "conservative" "ondemand" "powersave" "prediction_with_overhead" "prediction_wo_overhead") 
for i in "${GOVERNOR_FILES[@]}"
do 
    taskset 0xff ./filter_xpilot.py $i > $i
    mv $i $DATA_ODROID_PATH/$SAVED_FOLDER/xpilot_slice/$i
done

cd $DATA_ODROID_PATH
if [ $1 == "big" ] ; then
    sed -i -e 's/'"$PLOT_LITTLE"'/'"$PLOT_BIG"'/g' $DATA_ODROID_PATH/plot_both.py
    taskset 0xff ./plot_both.py big
    evince governors_big.pdf &
elif [ $1 == "little" ] ; then
    sed -i -e 's/'"$PLOT_BIG"'/'"$PLOT_LITTLE"'/g' $DATA_ODROID_PATH/plot_both.py
    taskset 0xff ./plot_both.py little
    evince governors_little.pdf &
fi

exit 0
