#!/bin/bash

source global.sh

echo "correct sweep??"
sleep 1

#enter password
if [ $ARCH_TYPE == "amd64" ] ; then 
	xdotool type 333
elif [ $ARCH_TYPE == "armhf" ] ; then
	xdotool type odroid
else 
	echo "unknown architecture"
	exit 1
fi
xdotool key KP_Enter

if [[ $# < 1 ]] ; then
    echo 'USAGE : ./run.sh [big/little/hetero]'
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
    SENSOR_ID="2-0040"
elif [ $1 == "little" ] ; then
    WHICH_CPU="cpu0"
    TASKSET_FLAG="0x0f"
    MAX_FREQ=1400000
    SENSOR_ID="2-0045"
elif [ $1 == "hetero" ] ; then
    WHICH_CPU="cpu0"
    TASKSET_FLAG="0xff"
    MAX_FREQ=2000000
    SENSOR_ID="2-0045"
fi

TEMP_PWD=`pwd`

#kill power_monitor process
PID_POWER_MONITOR=$(pgrep 'power_monitor')
sudo kill -9 $PID_POWER_MONITOR

#build power_monitor depends on big/little
if [ $1 != "hetero" ] ; then
    cd $POWER_MONITOR_PATH
    echo "entered "`pwd`
    ./power_monitor.sh $1
fi

for (( i=0; i<${#BENCH_NAME[@]}; i++ ));
do
    for (( j=0; j<${#SWEEP[@]}; j++ ));
    do

        cd $TEMP_PWD
        echo "entered "`pwd`

        #run power_monitor
        if [ $1 == "big" ] ; then
            rm -rf ../power_monitor/output_power.txt
            sudo taskset 0x0f ../power_monitor/power_monitor > ../power_monitor/output_power.txt &
        elif [ $1 == "little" ] ; then
            rm -rf ../power_monitor/output_power.txt
            sudo taskset 0xf0 ../power_monitor/power_monitor > ../power_monitor/output_power.txt &
        elif [ $1 == "hetero" ] ; then
            rm -rf ../power_monitor/output_power.txt
            sudo taskset 0xff ../power_monitor/power_monitor_both > ../power_monitor/output_power.txt &
        fi
        
        # ex) ./buildAll.sh [bench_index] [big/little] [prediction/oracle/pid dis/en] [sweep]
        # ex) ./runAll.sh [bench_index] [big/little] [govenors] [sweep]
        
        # 1. LINUX Governor
        echo ${SWEEP[$j]}
        taskset 0xff ./buildAll.sh $i $1 predict_dis ${SWEEP[$j]}
        ./runAll.sh $i $1 performance ${SWEEP[$j]}
        ./runAll.sh $i $1 interactive ${SWEEP[$j]}

        #disable convex
        sed -i -e 's/'"$CVX_ENABLED"'/'"$CVX_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

        # 2. Prediction offline
        taskset 0xff ./buildAll.sh $i $1 offline ${SWEEP[$j]}
        ./runAll.sh $i $1 offline ${SWEEP[$j]}
        
        # 3. Prediction online
        taskset 0xff ./buildAll.sh $i $1 online ${SWEEP[$j]}
        ./runAll.sh $i $1 online ${SWEEP[$j]}

        #kill power_monitor process
        sleep 30
        PID_POWER_MONITOR=$(pgrep 'power_monitor')
        sudo kill -9 $PID_POWER_MONITOR
        cp $POWER_MONITOR_PATH/output_power.txt $DATA_ODROID_PATH/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}

        #filter uzbl
#		if [ ${BENCH_NAME[$i]} == "uzbl" ] ; then
#           cd $DATA_ODROID_PATH
#            GOVERNOR_FILES=( "performance" "interactive" "cvx_with_overhead-100" "cvx_wo_overhead-100" "cvx_with_slice_only-100" "pid") 
#            for k in "${GOVERNOR_FILES[@]}"
#            do 
#                taskset 0xff ./filter_uzbl.py $1 ${SWEEP[$j]} $k > temp_uzbl
#                mv temp_uzbl $DATA_ODROID_PATH/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}/$k
#            done
#        fi       
    done

    # set SWEEP as 1
    sed -i -e 's/'"SWEEP (${SWEEP[$j-1]})"'/'"SWEEP (100)"'/g' $BENCH_PATH/$COMMON_FILE
 
done

#kill power_monitor process
sleep 3 
PID_POWER_MONITOR=$(pgrep 'power_monitor')
sudo kill -9 $PID_POWER_MONITOR

rm -rf sed*

echo "[ ./temp.sh done ]"
exit 0
