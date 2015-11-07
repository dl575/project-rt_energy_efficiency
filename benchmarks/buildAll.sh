#!/bin/bash

source global.sh


#Check the lengh of two arrays
if (( ${#SOURCE_FILES[@]} != ${#SOURCE_PATH[@]} )) ; then
    echo ${#SOURCE_FILES[@]}
    echo ${#SOURCE_PATH[@]}
    echo 'array size is different!'
    exit 1
fi

if [[ $# < 4 ]] ; then
    echo 'The number of arguments is wrong'
    exit 1
fi

if [ $2 != "big" ] && [ $2 != "little" ] && [ $2 != "hetero" ] ; then
    echo 'USAGE : ./buildAll.sh [N] [big/little/hetero] [policy] [sweep]'
    exit 1
fi

# set architecture depends on ARCH_TYPE
if [ $ARCH_TYPE == "amd64" ] ; then 
    sed -i -e 's/'"$ARCH_ARM_EN"'/'"$ARCH_ARM_DIS"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$ARCH_X86_DIS"'/'"$ARCH_X86_EN"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $ARCH_TYPE == "armhf" ] ; then
    sed -i -e 's/'"$ARCH_ARM_DIS"'/'"$ARCH_ARM_EN"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$ARCH_X86_EN"'/'"$ARCH_X86_DIS"'/g' $BENCH_PATH/$COMMON_FILE
else 
	echo "unknown architecture"
	exit 1
fi

# set core depends on argument 2
if [ $2 == "big" ] ; then
    sed -i -e 's/'"$CORE_LITTLE"'/'"$CORE_BIG"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$HETERO_ENABLED"'/'"$HETERO_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $2 == "little" ] ; then
    sed -i -e 's/'"$CORE_BIG"'/'"$CORE_LITTLE"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$HETERO_ENABLED"'/'"$HETERO_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $2 == "hetero" ] ; then
    sed -i -e 's/'"$CORE_BIG"'/'"$CORE_LITTLE"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$HETERO_DISABLED"'/'"$HETERO_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
fi

# set predictor
sed -i -e 's/'"$CVX_ENABLED"'/'"$CVX_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable DEBUG_EN
sed -i -e 's/'"$DEBUG_ENABLED"'/'"$DEBUG_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# enable DVFS_EN
if [ $ARCH_TYPE == "amd64" ] ; then 
	sed -i -e 's/'"$DVFS_ENABLED"'/'"$DVFS_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $ARCH_TYPE == "armhf" ] ; then
	sed -i -e 's/'"$DVFS_DISABLED"'/'"$DVFS_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
else 
	echo "unknown architecture"
	exit 1
fi


# enable DELAY_EN
sed -i -e 's/'"$DELAY_DISABLED"'/'"$DELAY_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable IDLE_EN
sed -i -e 's/'"$IDLE_ENABLED"'/'"$IDLE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable GET_PREDICT, GET_OVERHEAD, GET_DEADLINE
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_OVERHEAD_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_DEADLINE_ENABLED"'/'"$GET_DEADLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable all flags ralated to predict/oralce/pid
sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$SLICE_OVERHEAD_ONLY_ENABLED"'/'"$SLICE_OVERHEAD_ONLY_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$ORACLE_ENABLED"'/'"$ORACLE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$PID_ENABLED"'/'"$PID_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$PROACTIVE_ENABLED"'/'"$PROACTIVE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$ONLINE_ENABLED"'/'"$ONLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# set PREDICT_EN depends on argument 3
if [ $3 == "set_prediction_offline" ] ; then
    sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$ONLINE_ENABLED"'/'"$ONLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "set_prediction_online" ] ; then
    sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$ONLINE_DISABLED"'/'"$ONLINE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "predict_dis" ] ; then
    sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "predict_dis_idle" ] ; then
    sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
	sed -i -e 's/'"$IDLE_DISABLED"'/'"$IDLE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "overhead_en" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "overhead_en_idle" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
	  sed -i -e 's/'"$IDLE_DISABLED"'/'"$IDLE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "overhead_dis" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "slice_overhead_en" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$SLICE_OVERHEAD_ONLY_DISABLED"'/'"$SLICE_OVERHEAD_ONLY_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "oracle_en" ] ; then
    sed -i -e 's/'"$ORACLE_DISABLED"'/'"$ORACLE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "pid_en" ] ; then
    sed -i -e 's/'"$PID_DISABLED"'/'"$PID_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
	  sed -i -e 's/'"$ONLINE_ENABLED"'/'"$ONLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "pid_en_idle" ] ; then
    sed -i -e 's/'"$PID_DISABLED"'/'"$PID_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
	  sed -i -e 's/'"$IDLE_DISABLED"'/'"$IDLE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "proactive_en+overhead_en" ] ; then
    sed -i -e 's/'"$PROACTIVE_DISABLED"'/'"$PROACTIVE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "proactive_en+overhead_dis" ] ; then
    sed -i -e 's/'"$PROACTIVE_DISABLED"'/'"$PROACTIVE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "offline" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
	  sed -i -e 's/'"$ONLINE_ENABLED"'/'"$ONLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $3 == "online" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
	  sed -i -e 's/'"$ONLINE_DISABLED"'/'"$ONLINE_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
fi

function sweep {
    for (( i=0; i<${#SWEEP[@]}; i++ ));
    do
        sed -i -e 's/'"SWEEP (${SWEEP[$i]})"'/'"SWEEP ($1)"'/g' $BENCH_PATH/$COMMON_FILE
    done
}

sweep $4

function bench {
    for (( i=0; i<${#_ALL_BENCH_[@]}; i++ ));
    do
        sed -i -e 's/'"${_ALL_BENCH_[$i]} 1"'/'"${_ALL_BENCH_[$i]} 0"'/g' $BENCH_PATH/$COMMON_FILE
    done
    sed -i -e 's/'"$1 0"'/'"$1 1"'/g' $BENCH_PATH/$COMMON_FILE
}

bench ${_BENCH_FOR_DEFINE_[$1]}


cd $BENCH_PATH/${SOURCE_PATH[$1]}
echo "entered "$BENCH_PATH/${SOURCE_PATH[$1]}
#find . -type f | xargs -n 5 touch
if [ ${SOURCE_FILES[$1]} != "pocketsphinx/pocketsphinx-5prealpha/src/libpocketsphinx/pocketsphinx.c" ] ; then
	taskset 0xff make clean -j16
	taskset 0xff make -j16
fi

#Doing extra jobs (such as coyping binaries, fix_addresses)
if [ ${SOURCE_FILES[$1]} == "julius/julius-4.3.1/libjulius/src/recogmain.c" ] ; then
    echo "[julius] copy binary"
    rm -rf $BENCH_PATH/julius/julius-3.5.2-quickstart-linux/julius
    cp $BENCH_PATH/julius/julius-4.3.1/julius/julius $BENCH_PATH/julius/julius-3.5.2-quickstart-linux/julius
fi

#fix address for uzbl benchmarks
if [ ${SOURCE_FILES[$1]} == "uzbl/src/commands.c" ] ; then
    echo "[uzbl] ./fix_addresses.py"
    cd $BENCH_PATH/${SOURCE_PATH[$1]}
    taskset 0xff ./fix_addresses.py 
    taskset 0xff make -j16 
    taskset 0xff sudo make install 
elif [ ${SOURCE_FILES[$1]} == "pocketsphinx/pocketsphinx-5prealpha/src/libpocketsphinx/pocketsphinx.c" ] ; then
    echo "[pocketsphinx] make install"
    cd $BENCH_PATH/${SOURCE_PATH[$1]}
    sudo taskset 0xff make clean -j16
#    rm -rf autom4te.cache/
#    taskset 0xff ./autogen.sh
	sed -i -e 's/'"-g -O2 -Wall}"'/'"-g -O2 -Wall -D_GNU_SOURCE -std=c99}\nLIBS+=-lm"'/g' $BENCH_PATH/pocketsphinx/pocketsphinx-5prealpha/configure
    taskset 0xff ./configure --prefix=`pwd`/../install
#    vi $BENCH_PATH/pocketsphinx/pocketsphinx-5prealpha/configure
	sed -i -e 's/'"-g -O2 -Wall}"'/'"-g -O2 -Wall -D_GNU_SOURCE -std=c99}\nLIBS+=-lm"'/g' $BENCH_PATH/pocketsphinx/pocketsphinx-5prealpha/configure
#	taskset 0xff sudo make
#    vi $BENCH_PATH/pocketsphinx/pocketsphinx-5prealpha/configure
    taskset 0xff sudo make install 
fi


echo '[ build done ]'
exit 0


