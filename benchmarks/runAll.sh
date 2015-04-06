#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter
BENCHMARKS=(
"2048.c"
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
sleep 3 
if [[ $3 ]] ; then # $2 is specific governor(or prediction)
    cd $BENCH_PATH/${BENCHMARKS[$i]}
    echo "entered "`pwd`
    if [[ $4 ]] ; then # $3 is freq
        ./run.sh $2 $3 $4
    else
        ./run.sh $2 $3
    fi
else
    cd $BENCH_PATH/${BENCHMARKS[$i]}
    echo "entered "`pwd`
    ./run.sh $2
fi

echo "[ all done ]"
exit 1
