#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

BENCHMARKS=(
"mibench/office/stringsearch"
"mibench/security/sha"
"mibench/security/rijndael"
"xpilot/xpilot-4.5.5"
"julius/julius-3.5.2-quickstart-linux"
"2048.c"
"curseofwar"
)
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/

if [[ $# < 1 ]] ; then
    echo 'USAGE : ./run.sh big or ./run.sh little'
    exit 1
fi

if [ $2 != "big" -a $2 != "little" ] ; then
    echo 'USAGE : only big or little'
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
