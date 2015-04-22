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

if [ $2 != "big" -a $2 != "little" ] ; then
    echo 'USAGE : only big or little'
    exit 1
fi

if [ $3 != "predict_en" -a $3 != "predict_dis" ] ; then
    echo 'USAGE : only predict_en or predict_dis'
    exit 1
fi

#if [ $4 != "overhead_en" -a $4 != "overhead_dis" ] ; then
#    echo 'USAGE : only overhead_en or overhead_dis'
#    exit 1
#fi

# set deadline depends on argument 1
#if [ ${BENCH_NAME[$1]} == "2048_slice" ] ; then
#    sed -i -e 's/'"$D0_DISABLED"'/'"$D0_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    sed -i -e 's/'"$D17_ENABLED"'/'"$D17_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    sed -i -e 's/'"$D33_ENABLED"'/'"$D33_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#elif [ ${BENCH_NAME[$1]} == "2048_slice_17ms" ] ; then
#    sed -i -e 's/'"$D0_ENABLED"'/'"$D0_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    sed -i -e 's/'"$D17_DISABLED"'/'"$D17_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    sed -i -e 's/'"$D33_ENABLED"'/'"$D33_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#elif [ ${BENCH_NAME[$1]} == "2048_slice_17ms" ] ; then
#    sed -i -e 's/'"$D0_ENABLED"'/'"$D0_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    sed -i -e 's/'"$D17_ENABLED"'/'"$D17_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    sed -i -e 's/'"$D33_DISABLED"'/'"$D33_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
#fi

# set core depends on argument 2
if [ $2 == "big" ] ; then
    sed -i -e 's/'"$CORE_LITTLE"'/'"$CORE_BIG"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $2 == "little" ] ; then
    sed -i -e 's/'"$CORE_BIG"'/'"$CORE_LITTLE"'/g' $BENCH_PATH/$COMMON_FILE
fi

# disable DEBUG_EN
sed -i -e 's/'"$DEBUG_ENABLED"'/'"$DEBUG_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# enable DVFS_EN
sed -i -e 's/'"$DVFS_DISABLED"'/'"$DVFS_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

# enable DELAY_EN
sed -i -e 's/'"$DELAY_DISABLED"'/'"$DELAY_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable GET_PREDICT, GET_OVERHEAD, GET_DEADLINE
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_OVERHEAD_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_DEADLINE_ENABLED"'/'"$GET_DEADLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# set PREDICT_EN depends on argument 3
if [ $3 == "predict_en" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    if [ $4 == "overhead_en" ] ; then
#        sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    elif [ $4 == "overhead_dis" ] ; then
#        sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    fi
#    cd $BENCH_PATH/${SOURCE_PATH[$1]}
#    echo "entered "$BENCH_PATH/${SOURCE_PATH[$1]}
#    find . -type f | xargs -n 5 touch
#    taskset 0xff make clean -j16
#    taskset 0xff make -j16 
elif [ $3 == "predict_dis" ] ; then
    sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    if [ $4 == "overhead_en" ] ; then
#        sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    elif [ $4 == "overhead_dis" ] ; then
#        sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
#    fi
fi

function sweep {
    for (( i=0; i<${#SWEEP[@]}; i++ ));
    do
        sed -i -e 's/'"SWEEP (${SWEEP[$i]})"'/'"SWEEP ($1)"'/g' $BENCH_PATH/$COMMON_FILE
    done
}

sweep $4

cd $BENCH_PATH/${SOURCE_PATH[$1]}
echo "entered "$BENCH_PATH/${SOURCE_PATH[$1]}
find . -type f | xargs -n 5 touch
taskset 0xff make clean -j16
taskset 0xff make -j16

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
    taskset 0xff ./autogen.sh
    taskset 0xff ./configure --prefix=`pwd`/../install
    taskset 0xff sudo make install 
fi


echo '[ build done ]'
exit 1


