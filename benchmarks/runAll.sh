#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

BENCHMARKS=("mibench/office/stringsearch"
            "mibench/security/sha")
#BENCHMARKS=("mibench/security/sha"
#            "mibench/security/rijndael"
#            "mibench/office/stringsearch"
#            "xpilot/xpilot-4.5.5"
#            "julius/julius-3.5.2-quickstart-linux")
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/

if [[ $# < 1 ]] ; then
    echo 'USAGE : ./run.sh big or ./run.sh little'
    exit 1
fi

if [ $2 != "big_with_overhead" -a $2 != "big_wo_overhead" ] ; then
    echo 'USAGE : only big_with_overhead or big_wo_overhead'
    exit 1
fi
if [ $2 != "little_with_overhead" -a $2 = "little_wo_overhead" ] ; then
    echo 'USAGE : only little_with_overhead or little_wo_overhead'
    exit 1
fi

i=$1

if [[ $3 ]] ; then # $2 is specific governor(or prediction)
    sleep 10 
    cd $BENCH_PATH/${BENCHMARKS[$i]}
    echo "entered "`pwd`
    if [[ $4 ]] ; then # $3 is freq
        ./run.sh $2 $3 $4
    else
        ./run.sh $2 $3
    fi
    echo "[ all done ]"
    exit 1
fi

sleep 10
cd $BENCH_PATH/${BENCHMARKS[$i]}
echo "entered "`pwd`
./run.sh $2

echo "[ all done ]"
exit 1
