#!/bin/bash

BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/
SOURCE_FILES=("mibench/security/sha/sha_driver.c" 
            "mibench/office/stringsearch/pbmsrch_large.c"
            "mibench/security/rijndael/aesxam.c" 
            "xpilot/xpilot-4.5.5/src/server/server.c" 
            "julius/julius-4.3.1/libjulius/src/recogmain.c")
SOURCE_PATH=("mibench/security/sha" 
            "mibench/office/stringsearch"
            "mibench/security/rijndael"
            "xpilot/xpilot-4.5.5"
            "julius/julius-4.3.1")

PREDICT_ENABLED="PREDICT_EN 1"
PREDICT_DISABLED="PREDICT_EN 0"

BEFORE_MODIFIED="delay_time\*0.95"
AFTER_MODIFIED="delay_time\*0.90"

#Check the lengh of two arrays
if (( ${#SOURCE_FILES[@]} != ${#SOURCE_PATH[@]} )) ; then
    echo ${#SOURCE_FILES[@]}
    echo ${#SOURCE_PATH[@]}
    echo 'array size is different!'
    exit 1
fi

if [[ $# < 1 ]] ; then
    echo 'USAGE : ./buildAll.sh enable or ./buildAll.sh disable'
    exit 1
fi

if [ $1 != "enable" -a $1 != "disable" ] ; then
    echo 'USAGE : only enable or disable'
    exit 1
fi

for (( i=0; i<${#SOURCE_FILES[@]}; i++ ));
do
#    sed -i -e 's/'"$BEFORE_MODIFIED"'/'"$AFTER_MODIFIED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
    if [ $1 == "enable" ] ; then
        sed -i -e 's/'"$PREDICT_DISABLED"'/'"$PREDICT_ENABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
        cd $BENCH_PATH/${SOURCE_PATH[$i]}
        make
    elif [ $1 == "disable" ] ; then
        sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/${SOURCE_FILES[$i]}
        cd $BENCH_PATH/${SOURCE_PATH[$i]}
        make
    fi
done

#Doing extra jobs (such as coyping binaries)
cp $BENCH_PATH/julius/julius-4.3.1/julius/julius $BENCH_PATH/julius/julius-3.5.2-quickstart-linux/julius

echo '[ build done ]'
exit 1


