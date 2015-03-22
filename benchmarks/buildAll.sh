#!/bin/bash

BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/
COMMON_FILE=("common.h")
SOURCE_FILES=(
"2048.c/2048.c"
)
SOURCE_PATH=(
"2048.c"
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
#BEFORE_MODIFIED="delay_time\*0.95"
#AFTER_MODIFIED="delay_time\*0.90"

#Check the lengh of two arrays
if (( ${#SOURCE_FILES[@]} != ${#SOURCE_PATH[@]} )) ; then
    echo ${#SOURCE_FILES[@]}
    echo ${#SOURCE_PATH[@]}
    echo 'array size is different!'
    exit 1
fi

if [[ $# < 4 ]] ; then
    echo 'USAGE : ./buildAll.sh predict_en or ./buildAll.sh predict_dis'
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

if [ $4 != "overhead_en" -a $4 != "overhead_dis" ] ; then
    echo 'USAGE : only overhead_en or overhead_dis'
    exit 1
fi

# set core depends on argument 1
if [ $2 == "big" ] ; then
    sed -i -e 's/'"$CORE_LITTLE"'/'"$CORE_BIG"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $2 == "little" ] ; then
    sed -i -e 's/'"$CORE_BIG"'/'"$CORE_LITTLE"'/g' $BENCH_PATH/$COMMON_FILE
fi

# enable DVFS_EN
sed -i -e 's/'"$DVFS_DISABLED"'/'"$DVFS_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

# enable DELAY_EN
sed -i -e 's/'"$DELAY_DISABLED"'/'"$DELAY_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable GET_PREDICT, GET_OVERHEAD, GET_DEADLINE
sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_OVERHEAD_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$GET_DEADLINE_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# set PREDICT_EN, OVERHEAD_EN depends on argument 3, 4
if [ $3 == "predict_en" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    if [ $4 == "overhead_en" ] ; then
        sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    elif [ $4 == "overhead_dis" ] ; then
        sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    fi
    cd $BENCH_PATH/${SOURCE_PATH[$1]}
    echo "entered "$BENCH_PATH/${SOURCE_PATH[$1]}
    find . -type f | xargs -n 5 touch
    taskset 0xff make clean
    taskset 0xff make -j16 
elif [ $3 == "predict_dis" ] ; then
    sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    if [ $4 == "overhead_en" ] ; then
        sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
    elif [ $4 == "overhead_dis" ] ; then
        sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    fi
    cd $BENCH_PATH/${SOURCE_PATH[$1]}
    echo "entered "$BENCH_PATH/${SOURCE_PATH[$1]}
    find . -type f | xargs -n 5 touch
    taskset 0xff make clean
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
fi

echo '[ build done ]'
exit 1


