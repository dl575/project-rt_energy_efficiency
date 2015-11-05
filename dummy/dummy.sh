#!/bin/bash

source global.sh

if [[ $# < 1 ]] ; then
  echo 'USAGE : ./dummy.sh [big/little]'
  exit 1
fi

if [ $1 != "big" ] && [ $1 != "little" ] && [ $1 != "hetero" ] ; then
  echo 'USAGE : ./dummy.sh [big/little]'
  exit 1
fi

if [ $1 == "big" ] ; then
    WHICH_CPU="cpu7"
    TASKSET_FLAG="0x80"
    MAX_FREQ=2000000
    SENSOR_ID="3-0040"
elif [ $1 == "little" ] ; then
	if [ $ARCH_TYPE == "amd64" ] ; then 
    WHICH_CPU="cpu3"
    TASKSET_FLAG="0x08"
    MAX_FREQ=2534000
	elif [ $ARCH_TYPE == "armhf" ] ; then 
    WHICH_CPU="cpu3"
		TASKSET_FLAG="0x08"
		MAX_FREQ=1400000
	else 
		echo "unknown architecture"
		exit 1
	fi
  SENSOR_ID="3-0045"
elif [ $1 == "hetero" ] ; then
  WHICH_CPU="cpu3"
  TASKSET_FLAG="0x08"
  MAX_FREQ=1400000
  SENSOR_ID="3-0045"
fi

echo "[interference start]"
START=$(($(date +%s%N)/1000))
echo $START
#for (( j=0; j<5000; j++ ));
#do
#  taskset $TASKSET_FLAG echo "interference" > dummy.out
#  taskset $TASKSET_FLAG echo "interference" > dummy.out
#done
#rm dummy.out
cd compile_long
taskset $TASKSET_FLAG make -B
cd ../
echo "[interference end]"
END=$(($(date +%s%N)/1000))
echo $END
echo $(($END-$START))" us"
