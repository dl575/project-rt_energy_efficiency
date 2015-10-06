#!/bin/bash

#enter password
xdotool type odroid
xdotool key KP_Enter

source global.sh

if [[ $# < 2 ]] ; then
    echo 'USAGE : ./set_prediction [big/little] [conservative/cvx]'
    exit 1
fi

if [ $1 != "big" -a $1 != "little" ] ; then
    echo 'USAGE : ./set_prediction [big/little] [conservative/cvx]'
    exit 1
fi

if [ $2 != "conservative" -a $2 != "cvx" ] ; then
    echo 'USAGE : ./set_prediction [big/little] [conservative/cvx]'
    exit 1
fi

# set core depends on argument 1
if [ $1 == "big" ] ; then
    sed -i -e 's/'"$CORE_LITTLE"'/'"$CORE_BIG"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$HETERO_ENABLED"'/'"$HETERO_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $1 == "little" ] ; then
    sed -i -e 's/'"$CORE_BIG"'/'"$CORE_LITTLE"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$HETERO_ENABLED"'/'"$HETERO_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
fi

# set core depends on argument 2
if [ $2 == "conservative" ] ; then
    sed -i -e 's/'"$CVX_ENABLED"'/'"$CVX_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
elif [ $2 == "cvx" ] ; then
    sed -i -e 's/'"$CVX_DISABLED"'/'"$CVX_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE
fi

# disable DEBUG_EN
sed -i -e 's/'"$DEBUG_ENABLED"'/'"$DEBUG_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# enable DVFS_EN
sed -i -e 's/'"$DVFS_DISABLED"'/'"$DVFS_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable IDLE_EN
sed -i -e 's/'"$IDLE_ENABLED"'/'"$IDLE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

# disable all flags ralated to predict/oralce/pid
sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$OVERHEAD_ENABLED"'/'"$OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$ORACLE_ENABLED"'/'"$ORACLE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
sed -i -e 's/'"$PID_ENABLED"'/'"$PID_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

for (( i=0; i<${#BENCH_NAME[@]}; i++ ));
do
    function bench {
        for (( j=0; j<${#_ALL_BENCH_[@]}; j++ ));
        do
            sed -i -e 's/'"${_ALL_BENCH_[$j]} 1"'/'"${_ALL_BENCH_[$j]} 0"'/g' $BENCH_PATH/$COMMON_FILE
        done
        sed -i -e 's/'"$1 0"'/'"$1 1"'/g' $BENCH_PATH/$COMMON_FILE
    }
    bench ${_BENCH_FOR_DEFINE_[$i]}

    #---------------to get prediction equation--------------
    # get_predict enable, predict/delay/overhead disable
    sed -i -e 's/'"$GET_PREDICT_DISABLED"'/'"$GET_PREDICT_ENABLED"'/g' $BENCH_PATH/$COMMON_FILE

    sed -i -e 's/'"$PREDICT_ENABLED"'/'"$PREDICT_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$DELAY_ENABLED"'/'"$DELAY_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$GET_OVERHEAD_ENABLED"'/'"$GET_OVERHEAD_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE
    sed -i -e 's/'"$GET_DEADLINE_ENABLED"'/'"$GET_DEADLINE_DISABLED"'/g' $BENCH_PATH/$COMMON_FILE

    #move to benchamrk folder and build
    cd $BENCH_PATH/${SOURCE_PATH[$i]}
#    find . -type f | xargs -n 5 touch
    if [ ${SOURCE_FILES[$i]} != "pocketsphinx/pocketsphinx-5prealpha/src/libpocketsphinx/pocketsphinx.c" ] ; then
    	taskset 0xff make clean -j16
   		taskset 0xff make -j16
	fi

    #Doing extra jobs (such as coyping binaries, fix_addresses)
    if [ ${SOURCE_FILES[$i]} == "julius/julius-4.3.1/libjulius/src/recogmain.c" ] ; then
        echo "[julius] copy binary"
        rm -rf $BENCH_PATH/julius/julius-3.5.2-quickstart-linux/julius
        cp $BENCH_PATH/julius/julius-4.3.1/julius/julius $BENCH_PATH/julius/julius-3.5.2-quickstart-linux/julius
    fi

    #fix address for uzbl benchmarks
    if [ ${SOURCE_FILES[$i]} == "uzbl/src/commands.c" ] ; then
        echo "[uzbl] ./fix_addresses.py & make install"
        cd $BENCH_PATH/${SOURCE_PATH[$i]}
        taskset 0xff ./fix_addresses.py 
        taskset 0xff make -j16 
        taskset 0xff sudo make install 
    elif [ ${SOURCE_FILES[$i]} == "pocketsphinx/pocketsphinx-5prealpha/src/libpocketsphinx/pocketsphinx.c" ] ; then
        echo "[pocketsphinx] make install"
        cd $BENCH_PATH/${SOURCE_PATH[$i]}
    	sudo taskset 0xff make clean -j16
    	taskset 0xff ./configure --prefix=`pwd`/../install
        taskset 0xff sudo make install 
    elif [ ${SOURCE_FILES[$i]} == "curseofwar/main-sdl.c" ] ; then
        echo "[curseofwar] make SDL=yes"
        cd $BENCH_PATH/${SOURCE_PATH[$i]}
        taskset 0xff make SDL=yes
    fi

    #run bnechmark
    cd $BENCH_PATH/${BENCHMARKS[$i]}
	if [ ${SOURCE_FILES[$1]} == "curseofwar/main.c" ] ; then
    	./run_no_sdl.sh $1 performance temp_sample
	else
    	./run.sh $1 performance temp_sample
	fi

    cp $DVFS_SIM_PATH/data_odroid/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}-temp_sample/performance $DVFS_SIM_PATH/data_odroid/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}-temp_sample/temp.txt
    
    cp $DVFS_SIM_PATH/data_odroid/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}-temp_sample/performance $BENCH_PATH/${SOURCE_PATH[$i]}/M0.txt
    cp $DVFS_SIM_PATH/data_odroid/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}-temp_sample/performance $DVFS_SIM_PATH/data/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"0.txt"
    mv $DVFS_SIM_PATH/data_odroid/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}-temp_sample/performance $DVFS_SIM_PATH/data/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"1.txt"

    #filter xpilot_slice and uzbl
    if [ ${BENCH_NAME[$i]} == "xpilot_slice" ] ; then
        cd $DATA_ODROID_PATH
        taskset 0xff ./filter_xpilot.py $1 temp_sample temp.txt > temp
        cp temp $BENCH_PATH/${SOURCE_PATH[$i]}/M0.txt
        cp temp $DVFS_SIM_PATH/data/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"0.txt"
        mv temp $DVFS_SIM_PATH/data/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"1.txt"
    elif [ ${BENCH_NAME[$i]} == "uzbl" ] ; then
        cd $DATA_ODROID_PATH
        taskset 0xff ./filter_uzbl.py $1 temp_sample temp.txt > temp
        cp temp $BENCH_PATH/${SOURCE_PATH[$i]}/M0.txt
        cp temp $DVFS_SIM_PATH/data/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"0.txt"
        mv temp $DVFS_SIM_PATH/data/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}"1.txt"
    fi       

    rm -rf $DVFS_SIM_PATH/data_odroid/$1/${BENCH_NAME[$i]}/${BENCH_NAME[$i]}-temp_sample
done

cd $DVFS_SIM_PATH
taskset 0xff $DVFS_SIM_PATH/predict_times.py 
sleep 3
#cd $DVFS_SIM_PATH/lps
#taskset 0xff $DVFS_SIM_PATH/lps/gen_predictor.py
#cd $DVFS_SIM_PATH/pid
#taskset 0xff $DVFS_SIM_PATH/pid/gen_predictor.py
#cd $DVFS_SIM_PATH
#taskset 0xff $DVFS_SIM_PATH/gen_oracle_array.py
cd $DVFS_SIM_PATH/cvx
taskset 0xff $DVFS_SIM_PATH/cvx/gen_predictor.py

for (( i=0; i<${#BENCH_NAME[@]}; i++ ));
do
    echo "#if _"${BENCH_NAME[$i]}"_"
    cd $BENCH_PATH/${SOURCE_PATH[$i]}
    #run find_deadline.py script
    taskset 0xff $DVFS_SIM_PATH/data_odroid/find_deadline.py M0.txt
    echo "#endif"
done

exit 0
