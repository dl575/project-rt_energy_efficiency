#!/bin/bash

PROJECT_PATH=/home/odroid/project-rt_energy_efficiency/
POWER_MONITOR_PATH=$PROJECT_PATH/power_monitor/

RUN_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/mibench/security/sha_preread/
RUN_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/mibench/security/rijndael_preread/

xdotool type odroid
xdotool key KP_Enter

TEMP_PWD=`pwd`

#kill power_monitor process
PID_POWER_MONITOR=$(pgrep 'power_monitor')
sudo kill -9 $PID_POWER_MONITOR

#build power_monitor depends on big/little
cd $POWER_MONITOR_PATH
echo "entered "`pwd`
./power_monitor.sh little

cd $TEMP_PWD
echo "entered "`pwd`
rm -rf $POWER_MONITOR_PATH/output_power*.txt

for (( f=200000; f<1400000+1 ; f=f+100000 ));
do
  echo $f

  echo $f > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 
  sudo taskset 0xf0 $POWER_MONITOR_PATH/power_monitor > $f &

  cd $RUN_PATH

  taskset 0x01 $RUN_PATH/runme_slice.sh &
  taskset 0x02 $RUN_PATH/runme_slice.sh &
  taskset 0x03 $RUN_PATH/runme_slice.sh &
  taskset 0x04 $RUN_PATH/runme_slice.sh &
  
  sleep 10

  cd $TEMP_PWD
  PID_POWER_MONITOR=$(pgrep 'power_monitor')
  sudo kill -9 $PID_POWER_MONITOR

  PID_RUN_MONITOR=$(pgrep 'sha')
  sudo kill -9 $PID_RUN_MONITOR
done

echo 1400000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 
 
exit 0
