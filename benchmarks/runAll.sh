#!/bin/bash

source global.sh

if [[ $# < 4 ]] ; then
    echo 'The number of arguments is wrong'
    exit 1
fi

if [ $2 != "big" ] && [ $2 != "little" ] && [ $2 != "hetero" ] ; then
    echo 'USAGE : ./runAll.sh [N] [big/little/hetero] [governors] [sweep]'
    exit 1
fi

# ex) ./runAll.sh [bench_index] [big/little] [govenors] [sweep]
cd $BENCH_PATH/${BENCHMARKS[$1]}
echo "entered "`pwd`
./run.sh $2 $3 $4


#sleep 3 
#if [[ $3 ]] ; then # $2 is specific governor(or prediction)
#    cd $BENCH_PATH/${BENCHMARKS[$1]}
#    echo "entered "`pwd`
#    if [[ $4 ]] ; then # $3 is freq
#        ./run.sh $2 $3 $4
#    else
#        ./run.sh $2 $3
#    fi
#else
#    cd $BENCH_PATH/${BENCHMARKS[$1]}
#    echo "entered "`pwd`
#    ./run.sh $2
##fi

#echo "[ all done ]"
exit 0
