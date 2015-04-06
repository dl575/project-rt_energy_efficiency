#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

if [[ $# < 1 ]] ; then
    echo 'USAGE : ./run.sh big or ./run.sh little'
    exit 1
fi

if [ $1 != "big" -a $1 != "little" ] ; then
    echo 'USAGE : only big or little'
    exit 1
fi

DVFS_SIM_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/
COMMON_FILE=("common.h")
SOURCE_FILES=("2048.c/2048.c")
SOURCE_PATH=("2048.c") 
BENCH_NAME=("2048_slice")

CORE_BIG="CORE 1"
CORE_LITTLE="CORE 0"
PREDICT_ENABLED="PREDICT_EN 1"
PREDICT_DISABLED="PREDICT_EN 0"
OVERHEAD_ENABLED="OVERHEAD_EN 1"
OVERHEAD_DISABLED="OVERHEAD_EN 0"
GET_PREDICT_ENABLED="GET_PREDICT 1"
GET_PREDICT_DISABLED="GET_PREDICT 0"
GET_DEADLINE_ENABLED="GET_DEADLINE 1"
GET_DEADLINE_DISABLED="GET_DEADLINE 0"
GET_OVERHEAD_ENABLED="GET_OVERHEAD 1"
GET_OVERHEAD_DISABLED="GET_OVERHEAD 0"
DELAY_ENABLED="DELAY_EN 1"
DELAY_DISABLED="DELAY_EN 0"
DVFS_ENABLED="DVFS_EN 1"
DVFS_DISABLED="DVFS_EN 0"

# set core depends on argument 1
if [ $1 == "big" ] ; then
    sed -i -e 's/'"$CORE_LITTLE"'/'"$CORE_BIG"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $1 == "little" ] ; then
    sed -i -e 's/'"$CORE_BIG"'/'"$CORE_LITTLE"'/g' $BENCH_PATH/$COMMON_FILE
fi

# enable DVFS_EN
sed -i -e 's/'"$DVFS_DISABLED"'/'"$DVFS_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

#---------------to get execution deadline--------------
# GET_DEADLINE enable, others disable, run performance
sed -i -e 's/'"$GET_DEADLINE_DISABLED"'/'"$GET_DEADLINE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_OVERHEAD_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

#move to benchamrk folder and build
cd $BENCH_PATH/$SOURCE_PATH
find . -type f | xargs -n 5 touch
taskset 0xff make clean
taskset 0xff make -j16

./run.sh $1 temp_sample
mv $DVFS_SIM_PATH/data_odroid/$1/$BENCH_NAME/temp_sample $BENCH_PATH/$SOURCE_PATH/M0.txt

#---------------to get overhead deadline--------------
# GET_OVERHEAD enable, others disable, run performance
sed -i -e 's/'"$GET_OVERHEAD_DISABLED"'/'"$GET_OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

sed -i -e 's/'"$GET_DEADLINE_ENABLED"'/'"$GET_DEADLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

#move to benchamrk folder and build
cd $BENCH_PATH/$SOURCE_PATH
find . -type f | xargs -n 5 touch
taskset 0xff make clean
taskset 0xff make -j16

./run.sh $1 temp_sample
mv $DVFS_SIM_PATH/data_odroid/$1/$BENCH_NAME/temp_sample $BENCH_PATH/$SOURCE_PATH/M1M2.txt

#run find_deadline.py script
taskset 0xff $DVFS_SIM_PATH/data_odroid/find_deadline.py M0.txt M1M2.txt

exit 0
