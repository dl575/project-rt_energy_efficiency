#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

source global.sh

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
    SENSOR_ID="3-0040"
elif [ $1 == "little" ] ; then
    WHICH_CPU="cpu0"
    TASKSET_FLAG="0x0f"
    MAX_FREQ=1400000
    SENSOR_ID="3-0045"
elif [ $1 == "hetero" ] ; then
    WHICH_CPU="cpu0"
    TASKSET_FLAG="0xff"
    MAX_FREQ=2000000
    SENSOR_ID="3-0045"
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
        # MUST RUN as this order. 
        # 1. LINUX Governor (performance > interactive > conservative > ondemand > powersave )
        # > 2. Prediction with overhead > 3. Prediction w/o overhead > 4. Oracle > 5. PID
        # ( 2. convex with overhead > 3. convex with overhead)
        
        # ex) ./buildAll.sh [bench_index] [big/little] [prediction/oracle/pid dis/en] [sweep]
        # ex) ./runAll.sh [bench_index] [big/little] [govenors] [sweep]
        
        # 1. LINUX Governor
        echo ${SWEEP[$j]}
#        taskset 0xff ./buildAll.sh $i $1 predict_dis ${SWEEP[$j]}
#        ./runAll.sh $i $1 performance ${SWEEP[$j]}
#        ./runAll.sh $i $1 interactive ${SWEEP[$j]}
        #./runAll.sh $i $1 conservative ${SWEEP[$j]}
        #./runAll.sh $i $1 ondemand ${SWEEP[$j]}
        #./runAll.sh $i $1 powersave ${SWEEP[$j]}

        #enable convex
        sed -i -e 's/'"$CVX_DISABLED"'/'"$CVX_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
        # 2. convex  with overhead
        taskset 0xff ./buildAll.sh $i $1 overhead_en ${SWEEP[$j]}
        ./runAll.sh $i $1 cvx_with_overhead ${SWEEP[$j]}

        # 3. convex wo overhead
#        taskset 0xff ./buildAll.sh $i $1 overhead_dis ${SWEEP[$j]}
#        ./runAll.sh $i $1 cvx_wo_overhead ${SWEEP[$j]}

        # 3.5 convex with slice only
#        taskset 0xff ./buildAll.sh $i $1 slice_overhead_en ${SWEEP[$j]}
#        ./runAll.sh $i $1 cvx_with_slice_only-100 ${SWEEP[$j]}

        # 4. Oralce
#        if [[ ${BENCH_NAME[$i]} != "uzbl" ]] && [[ ${BENCH_NAME[$i]} != "xpilot_slice" ]] ; then
#            taskset 0xff ./buildAll.sh $i $1 oracle_en ${SWEEP[$j]}
#            ./runAll.sh $i $1 oracle ${SWEEP[$j]}
#        fi
 
        # 5. PID
#        taskset 0xff ./buildAll.sh $i $1 pid_en ${SWEEP[$j]}
#        ./runAll.sh $i $1 pid ${SWEEP[$j]}

        # 6. PROACTIVE with overhead
#        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_en ${SWEEP[$j]}
#        ./runAll.sh $i $1 proactive_with_overhead-W3 ${SWEEP[$j]}
        
        # 7. PROACTIVE wo overhead
#        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_dis ${SWEEP[$j]}
#        ./runAll.sh $i $1 proactive_wo_overhead-W3 ${SWEEP[$j]}

        # 6. PROACTIVE with overhead
        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_en ${SWEEP[$j]}
        ./runAll.sh $i $1 proactive_with_overhead-W5 ${SWEEP[$j]}
        
        # 7. PROACTIVE wo overhead
#        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_dis ${SWEEP[$j]}
#        ./runAll.sh $i $1 proactive_wo_overhead-W5 ${SWEEP[$j]}
        # 6. PROACTIVE with overhead
#        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_en ${SWEEP[$j]}
#        ./runAll.sh $i $1 proactive_with_overhead-W10 ${SWEEP[$j]}
        
        # 7. PROACTIVE wo overhead
#        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_dis ${SWEEP[$j]}
#        ./runAll.sh $i $1 proactive_wo_overhead-W10 ${SWEEP[$j]}

        # 6. PROACTIVE with overhead
#        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_en ${SWEEP[$j]}
#        ./runAll.sh $i $1 proactive_with_overhead-W20 ${SWEEP[$j]}
        
        # 7. PROACTIVE wo overhead
#        taskset 0xff ./buildAll.sh $i $1 proactive_en+overhead_dis ${SWEEP[$j]}
#        ./runAll.sh $i $1 proactive_wo_overhead-W20 ${SWEEP[$j]}


        #kill power_monitor process
        sleep 10 
        PID_POWER_MONITOR=$(pgrep 'power_monitor')
        sudo kill -9 $PID_POWER_MONITOR
        cp $POWER_MONITOR_PATH/output_power.txt $DATA_ODROID_PATH/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}

        #filter uzbl
        if [ ${BENCH_NAME[$i]} == "uzbl" ] ; then
            cd $DATA_ODROID_PATH
            GOVERNOR_FILES=( "performance" "interactive" "cvx_with_overhead-100" "cvx_wo_overhead-100" "cvx_with_slice_only-100" "pid") 
            for k in "${GOVERNOR_FILES[@]}"
            do 
                taskset 0xff ./filter_uzbl.py $1 ${SWEEP[$j]} $k > temp_uzbl
                mv temp_uzbl $DATA_ODROID_PATH/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}/$k
            done
        fi       
    done

    #set SWEEP as 1
#    sed -i -e 's/'"SWEEP (${SWEEP[$j-1]})"'/'"SWEEP (1)"'/g' $BENCH_PATH/$COMMON_FILE
 
   #draw plot
    cd $DATA_ODROID_PATH
    if [ $1 == "big" ] ; then
        sed -i -e 's/'"$PLOT_LITTLE"'/'"$PLOT_BIG"'/g' $DATA_ODROID_PATH/plot_both.py
        taskset 0xff ./plot_energy.py big ${BENCH_NAME[$i]}
    elif [ $1 == "little" ] ; then
        sed -i -e 's/'"$PLOT_BIG"'/'"$PLOT_LITTLE"'/g' $DATA_ODROID_PATH/plot_both.py
        taskset 0xff ./plot_energy.py little ${BENCH_NAME[$i]}
#        taskset 0xff ./plot_both.py little ${BENCH_NAME[$i]}
    fi

done

#kill power_monitor process
sleep 3 
PID_POWER_MONITOR=$(pgrep 'power_monitor')
sudo kill -9 $PID_POWER_MONITOR

rm -rf sed*

echo "[ ./temp.sh done ]"
exit 0
