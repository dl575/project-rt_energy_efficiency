#!/bin/bash

BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/

SOURCE_FILES=(
"mibench/office/stringsearch/pbmsrch_large.c"
"mibench/security/sha/sha_driver.c"
"mibench/security/rijndael/aesxam.c"
"xpilot/xpilot-4.5.5/src/server/server.c"
"julius/julius-4.3.1/libjulius/src/recogmain.c"
"2048.c/2048.c"
"curseofwar/main.c"
)
SOURCE_PATH=(
"mibench/office/stringsearch"
"mibench/security/sha"
"mibench/security/rijndael"
"xpilot/xpilot-4.5.5"
"julius/julius-4.3.1"
"2048.c"
"curseofwar"
)

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
#BEFORE_MODIFIED="delay_time\*0.95"
#AFTER_MODIFIED="delay_time\*0.90"

#Check the lengh of two arrays
if (( ${#SOURCE_FILES[@]} != ${#SOURCE_PATH[@]} )) ; then
    echo ${#SOURCE_FILES[@]}
    echo ${#SOURCE_PATH[@]}
    echo 'array size is different!'
    exit 1
fi

if [[ $# < 3 ]] ; then
    echo 'USAGE : ./buildAll.sh predict_en or ./buildAll.sh predict_dis'
    exit 1
fi

if [ $2 != "predict_en" -a $2 != "predict_dis" ] ; then
    echo 'USAGE : only predict_en or predict_dis'
    exit 1
fi

if [ $3 != "overhead_en" -a $3 != "overhead_dis" ] ; then
    echo 'USAGE : only overhead_en or overhead_dis'
    exit 1
fi

i=$1
# Enable delay, diable get_predict
#sed -i -e 's/'"$DELAY_DISABLED"'/'"$DELAY_ENABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}

sed -i -e 's/'"$GET_PREDICT_ENABLED"'/'"$GET_PREDICT_DISABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
sed -i -e 's/'"$GET_OVERHEAD_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
#    sed -i -e 's/'"$BEFORE_MODIFIED"'/'"$AFTER_MODIFIED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
if [ $2 == "predict_en" ] ; then
    sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
    if [ $3 == "overhead_en" ] ; then
        sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
    elif [ $3 == "overhead_dis" ] ; then
        sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
    fi
    cd $BENCH_PATH/${SOURCE_PATH[$i]}
    echo "entered "$BENCH_PATH/${SOURCE_PATH[$i]}
    find . -type f | xargs -n 5 touch
    taskset 0xff make clean
    taskset 0xff make -j16 
elif [ $2 == "predict_dis" ] ; then
    sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
    if [ $3 == "overhead_en" ] ; then
        sed -i -e 's/'"$OVERHEAD_DISABLED"'/'"$OVERHEAD_ENABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
    elif [ $3 == "overhead_dis" ] ; then
        sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
    fi
    cd $BENCH_PATH/${SOURCE_PATH[$i]}
    echo "entered "$BENCH_PATH/${SOURCE_PATH[$i]}
    find . -type f | xargs -n 5 touch
    taskset 0xff make clean
    taskset 0xff make -j16
fi

#Doing extra jobs (such as coyping binaries)
rm -rf $BENCH_PATH/julius/julius-3.5.2-quickstart-linux/julius
cp $BENCH_PATH/julius/julius-4.3.1/julius/julius $BENCH_PATH/julius/julius-3.5.2-quickstart-linux/julius

echo '[ build done ]'
exit 1


