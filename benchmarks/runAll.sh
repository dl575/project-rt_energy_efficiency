#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

#BENCHMARKS=("mibench/office/stringsearch" "mibench/security/sha" "mibench/security/rijndael" )
#BENCHMARKS=("mibench/office/stringsearch" "xpilot/xpilot-4.5.5" "mibench/security/sha" "mibench/security/rijndael" "julius/julius-4.3.1")
BENCHMARKS=("xpilot/xpilot-4.5.5")
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/

if [[ $# < 1 ]] ; then
    echo 'USAGE : ./run.sh big or ./run.sh little'
    exit 1
fi

if [ $1 != "big" -a $1 != "little" ] ; then
    echo 'USAGE : only big or little'
    exit 1
fi

if [[ $2 ]] ; then
    for i in "${BENCHMARKS[@]}"
    do 
        cd $BENCH_PATH/$i
        echo "entered "`pwd`
        ./run.sh $1 $2
    done
    echo "[ all done ]"
    exit 1
fi

for i in "${BENCHMARKS[@]}"
do 
    cd $BENCH_PATH/$i
    echo "entered "`pwd`
    ./run.sh $1
done
echo "[ all done ]"
exit 1
