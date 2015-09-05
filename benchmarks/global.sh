DATA_ODROID_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/data_odroid
POWER_MONITOR_PATH=/home/odroid/project-rt_energy_efficiency/power_monitor
TEMP_MONITOR_PATH=/home/odroid/project-rt_energy_efficiency/temp_monitor
BENCH_PATH=/home/odroid/project-rt_energy_efficiency/benchmarks/
DVFS_SIM_PATH=/home/odroid/project-rt_energy_efficiency/dvfs_sim/
COMMON_FILE=("my_common.h")

CORE_BIG="CORE 1"
CORE_LITTLE="CORE 0"
HETERO_ENABLED="HETERO_EN 1"
HETERO_DISABLED="HETERO_EN 0"
PREDICT_ENABLED="PREDICT_EN 1"
PREDICT_DISABLED="PREDICT_EN 0"
CVX_ENABLED="CVX_EN 1"
CVX_DISABLED="CVX_EN 0"
OVERHEAD_ENABLED="OVERHEAD_EN 1"
OVERHEAD_DISABLED="OVERHEAD_EN 0"
SLICE_OVERHEAD_ONLY_ENABLED="SLICE_OVERHEAD_ONLY_EN 1"
SLICE_OVERHEAD_ONLY_DISABLED="SLICE_OVERHEAD_ONLY_EN 0"
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

ORACLE_ENABLED="ORACLE_EN 1"
ORACLE_DISABLED="ORACLE_EN 0"
PID_ENABLED="PID_EN 1"
PID_DISABLED="PID_EN 0"
PROACTIVE_ENABLED="PROACTIVE_EN 1"
PROACTIVE_DISABLED="PROACTIVE_EN 0"

D0_DISABLED="DEADLINE_DEFAULT 0"
D0_ENABLED="DEADLINE_DEFAULT 1"
D0_DISABLED="DEADLINE_DEFAULT 0"
D17_ENABLED="DEADLINE_17MS 1"
D17_DISABLED="DEADLINE_17MS 0"
D33_ENABLED="DEADLINE_33MS 1"
D33_DISABLED="DEADLINE_33MS 0"
#BEFORE_MODIFIED="delay_time\*0.95"
#AFTER_MODIFIED="delay_time\*0.90"

#SWEEP=("60" "80" "100" "120" "140")
#SWEEP=("20" "40" "60" "80" "100" "120" "140" "160" "180")
#SWEEP=("100")
#SWEEP=("with_fan")
SWEEP=("60" "80" "100")
#SWEEP=("4000000")
_ALL_BENCH_=(
"_stringsearch_"
"_sha_preread_"
"_rijndael_preread_"
"_xpilot_slice_"
"_2048_slice_"
"_curseofwar_slice_sdl_"
"_ldecode_"
"_pocketsphinx_"
"_uzbl_"
)

#:53,94s/^/#
#:53,94s/^#/
#SOURCE_FILES=(
#"pocketsphinx/pocketsphinx-5prealpha/src/libpocketsphinx/pocketsphinx.c"
#)
#SOURCE_PATH=(
#"pocketsphinx/pocketsphinx-5prealpha"
#)
#BENCHMARKS=(
#"pocketsphinx/test"
#)
#BENCH_NAME=(
#"pocketsphinx"
#)
#_BENCH_FOR_DEFINE_=(
#"_pocketsphinx_"
#)

SOURCE_FILES=(
"mibench/security/sha_preread/sha_driver.c"
)
SOURCE_PATH=(
"mibench/security/sha_preread"
)
BENCHMARKS=(
"mibench/security/sha_preread"
)
BENCH_NAME=(
"sha_preread"
)
_BENCH_FOR_DEFINE_=(
"_sha_preread_"
)

#SOURCE_FILES=(
#"curseofwar/main-sdl.c"
#)
#SOURCE_PATH=(
#"curseofwar"
#)
#BENCHMARKS=(
#"curseofwar"
#)
#BENCH_NAME=(
#"curseofwar_slice_sdl"
#)
#_BENCH_FOR_DEFINE_=(
#"_curseofwar_slice_sdl_"
#)

#SOURCE_FILES=(
#"mibench/office/stringsearch/pbmsrch_large.c"
#"mibench/security/sha_preread/sha_driver.c"
#"mibench/security/rijndael_preread/aesxam.c"
#"2048.c/2048.c"
#"curseofwar/main-sdl.c"
#"xpilot/xpilot-4.5.5/src/server/server.c"
#"ldecode/src/image.c"
#"pocketsphinx/pocketsphinx-5prealpha/src/libpocketsphinx/pocketsphinx.c"
#"uzbl/src/commands.c"
#)
#SOURCE_PATH=(
#"mibench/office/stringsearch"
#"mibench/security/sha_preread"
#"mibench/security/rijndael_preread"
#"2048.c"
#"curseofwar"
#"xpilot/xpilot-4.5.5"
#"ldecode"
#"pocketsphinx/pocketsphinx-5prealpha"
#"uzbl"
#)
#BENCHMARKS=(
#"mibench/office/stringsearch"
#"mibench/security/sha_preread"
#"mibench/security/rijndael_preread"
#"2048.c"
#"curseofwar"
#"xpilot/xpilot-4.5.5"
#"ldecode"
#"pocketsphinx/test"
#"uzbl"
#)
#BENCH_NAME=(
#"stringsearch"
#"sha_preread"
#"rijndael_preread"
#"2048_slice"
#"curseofwar_slice_sdl"
#"xpilot_slice"
#"ldecode"
#"pocketsphinx"
#"uzbl"
#)
#_BENCH_FOR_DEFINE_=(
#"_stringsearch_"
#"_sha_preread_"
#"_rijndael_preread_"
#"_2048_slice_"
#"_curseofwar_slice_sdl_"
#"_xpilot_slice_"
#"_ldecode_"
#"_pocketsphinx_"
#"_uzbl_"
#)

#SOURCE_FILES=(
#"ldecode/src/image.c"
#)
#SOURCE_PATH=(
#"ldecode"
#)
#BENCHMARKS=(
#"ldecode"
#)
#BENCH_NAME=(
#"ldecode"
#)
#_BENCH_FOR_DEFINE_=(
#"_ldecode_"
#)

