DATA_ODROID_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/data_odroid
POWER_MONITOR_PATH=/home/odroid/project-rt_energy_efficiency/power_monitor
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/
DVFS_SIM_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/
COMMON_FILE=("my_common.h")

CORE_BIG="CORE 1"
CORE_LITTLE="CORE 0"
PREDICT_ENABLED="PREDICT_EN 1"
PREDICT_DISABLED="PREDICT_EN 0"
#OVERHEAD_ENABLED="OVERHEAD_EN 1"
#OVERHEAD_DISABLED="OVERHEAD_EN 0"
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
DEBUG_ENABLED="DEBUG_EN 1"
DEBUG_DISABLED="DEBUG_EN 0"

D0_DISABLED="DEADLINE_DEFAULT 0"
D0_ENABLED="DEADLINE_DEFAULT 1"
D0_DISABLED="DEADLINE_DEFAULT 0"
D17_ENABLED="DEADLINE_17MS 1"
D17_DISABLED="DEADLINE_17MS 0"
D33_ENABLED="DEADLINE_33MS 1"
D33_DISABLED="DEADLINE_33MS 0"
#BEFORE_MODIFIED="delay_time\*0.95"
#AFTER_MODIFIED="delay_time\*0.90"

SWEEP=("20" "40" "60" "80" "100" "120" "140" "160" "180")

#:53,94s/^/#
#:53,94s/^#/
SOURCE_FILES=(
"mibench/office/stringsearch/pbmsrch_large.c"
"mibench/security/sha/sha_driver.c"
"mibench/security/rijndael/aesxam.c"
"xpilot/xpilot-4.5.5/src/server/server.c"
"julius/julius-4.3.1/libjulius/src/recogmain.c"
"2048.c/2048.c"
"curseofwar/main.c"
"uzbl/src/commands.c"
)
SOURCE_PATH=(
"pocketsphinx/pocketsphinx-5prealpha"
"mibench/office/stringsearch"
"mibench/security/sha"
"mibench/security/rijndael"
"xpilot/xpilot-4.5.5"
"julius/julius-4.3.1"
"2048.c"
"curseofwar"
"uzbl"
)
BENCHMARKS=(
"pocketsphinx/test"
"mibench/office/stringsearch"
"mibench/security/sha"
"mibench/security/rijndael"
"xpilot/xpilot-4.5.5"
"julius/julius-3.5.2-quickstart-linux"
"2048.c"
"curseofwar"
"uzbl"
)
BENCH_NAME=(
"pocketsphinx"
"stringsearch"
"sha"
"rijndael"
"xpilot_slice"
"julius_slice"
"2048_slice"
"curseofwar_slice"
"uzbl"
)
