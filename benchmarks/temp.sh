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

if [[ $# < 2 ]] ; then
    echo 'USAGE : ./run.sh [big/little/hetero] [no_dummy/dummy_0/dummy_1]'
    exit 1
fi

if [ $1 != "big" ] && [ $1 != "little" ] && [ $1 != "hetero" ] ; then
    echo 'USAGE : ./run.sh [big/little/hetero]'
    exit 1
fi

if [ $2 == "dummy_0" ] ; then
  sed -i -e 's/'"DUMMY=0"'/'"DUMMY=1"'/g' $BENCH_PATH/run.sh
  sed -i -e 's/'"DUMMY_LEVEL=1"'/'"DUMMY_LEVEL=0"'/g' $BENCH_PATH/run.sh
elif [ $2 == "dummy_1" ] ; then
  sed -i -e 's/'"DUMMY=0"'/'"DUMMY=1"'/g' $BENCH_PATH/run.sh
  sed -i -e 's/'"DUMMY_LEVEL=0"'/'"DUMMY_LEVEL=1"'/g' $BENCH_PATH/run.sh
else
  sed -i -e 's/'"DUMMY=1"'/'"DUMMY=0"'/g' $BENCH_PATH/run.sh
  sed -i -e 's/'"DUMMY_LEVEL=1"'/'"DUMMY_LEVEL=0"'/g' $BENCH_PATH/run.sh
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

    if [ $ARCH_TYPE == "armhf" ] ; then 
      #run power_monitor
      if [ $1 == "big" ] ; then
        rm -rf ../power_monitor/output_power*.txt
        sudo taskset 0x0f ../power_monitor/power_monitor > ../power_monitor/output_power.txt &
      elif [ $1 == "little" ] ; then
        rm -rf ../power_monitor/output_power*.txt
        sudo taskset 0xf0 ../power_monitor/power_monitor > ../power_monitor/output_power.txt &
      elif [ $1 == "hetero" ] ; then
        rm -rf ../power_monitor/output_power*.txt
        sudo taskset 0xff ../power_monitor/power_monitor_both > ../power_monitor/output_power_hetero.txt &
      fi
    fi
      
    # ex) ./buildAll.sh [bench_index] [big/little] [prediction/oracle/pid dis/en] [sweep]
    # ex) ./runAll.sh [bench_index] [big/little] [govenors] [sweep]
    
    if [ $1 != "hetero" ] ; then
      # 1. LINUX Governor
      echo ${SWEEP[$j]}
      taskset 0xff ./buildAll.sh $i $1 predict_dis ${SWEEP[$j]}
      ./runAll.sh $i $1 performance ${SWEEP[$j]}
      ./runAll.sh $i $1 interactive ${SWEEP[$j]}

      # 2. PID
      taskset 0xff ./buildAll.sh $i $1 pid_en ${SWEEP[$j]}
      ./runAll.sh $i $1 pid ${SWEEP[$j]}

#    #disable convex
#    sed -i -e 's/'"$CVX_ENABLED"'/'"$CVX_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    # 3. Prediction offline with no penalty
#    taskset 0xff ./buildAll.sh $i $1 offline ${SWEEP[$j]}
#    ./runAll.sh $i $1 offline-no-penalty ${SWEEP[$j]}
    
      #enable convex
      sed -i -e 's/'"$CVX_DISABLED"'/'"$CVX_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
      # 4. Prediction offline with under prediction penalty
      taskset 0xff ./buildAll.sh $i $1 offline ${SWEEP[$j]}
      ./runAll.sh $i $1 offline-under-penalty ${SWEEP[$j]}

      # 5. Prediction online with no penalty
      taskset 0xff ./buildAll.sh $i $1 online ${SWEEP[$j]}
      ./runAll.sh $i $1 online ${SWEEP[$j]}

    elif [ $1 == "hetero" ] ; then
      taskset 0xff ./buildAll.sh $i $1 online+hetero ${SWEEP[$j]}
      ./runAll.sh $i $1 online+hetero ${SWEEP[$j]}
    fi

    #kill power_monitor process
    if [ $ARCH_TYPE == "armhf" ] ; then 
      sleep 30
      PID_POWER_MONITOR=$(pgrep 'power_monitor')
      sudo kill -9 $PID_POWER_MONITOR
      cp $POWER_MONITOR_PATH/output_power*.txt $DATA_ODROID_PATH/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}
      if [ $1 == "hetero" ] ; then
        cp $POWER_MONITOR_PATH/output_power*.txt $DATA_ODROID_PATH/little/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}
        cp $DATA_ODROID_PATH/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}/*hetero* $DATA_ODROID_PATH/little/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"-"${SWEEP[$j]}
      fi
    fi
  done

  # set SWEEP as 1
  sed -i -e 's/'"SWEEP (${SWEEP[$j-1]})"'/'"SWEEP (100)"'/g' $BENCH_PATH/$COMMON_FILE
 
done

#kill power_monitor process
if [ $ARCH_TYPE == "armhf" ] ; then 
  sleep 3 
  PID_POWER_MONITOR=$(pgrep 'power_monitor')
  sudo kill -9 $PID_POWER_MONITOR
fi

rm -rf sed*

echo "[ ./temp.sh done ]"
exit 0
