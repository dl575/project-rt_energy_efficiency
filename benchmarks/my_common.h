/* This file includes common functions which some of all benchmarks use. */

#ifndef __MY_COMMON_H__
#define __MY_COMMON_H__

#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <sched.h>
#include "timing.h"
#include "deadline.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <stddef.h>
#include <sys/types.h>

//constant
#define ERROR_DEFINE -1
#define AVG_DVFS_TIME (double)0
#define MARGIN (double)1.1

//manually set below
#define CORE 0 //0:LITTLE, 1:big
#define HETERO_EN 0 //0:use only one core, 1:use both cores

#define DELAY_EN 0 //0:delay off, 1:delay on
#define IDLE_EN 0 //0:idle off, 1:idle on

#define GET_PREDICT 1 //to get prediction equation
#define GET_OVERHEAD 0 // to get execution deadline
#define GET_DEADLINE 0 //to get overhead deadline
#define PREDICT_EN 0 //0:prediction off, 1:prediction on
#define CVX_EN 0 //0:prediction off, 1:prediction on
#define OVERHEAD_EN 0 //0:dvfs+slice overhead off, 1:dvfs+slice overhead on
#define SLICE_OVERHEAD_ONLY_EN 0 //0:dvfs overhead off, 1:dvfs overhead on
#define ORACLE_EN 0 //0:oracle off, 1:oracle on
#define PID_EN 0 //0:pid off, 1:pid on
#define PROACTIVE_EN 0 //0:proactive dvfs off, 1:proactvie dvfs on

#define WINDOW_SIZE (5) //window size

#define DEBUG_EN 0 //debug information print on/off

#define SWEEP (100) //sweep deadline (e.g, if 90, deadline*0.9)
#define CVX_COEFF (100) //cvx coefficient
#define LASSO_COEFF (0) //lasso coefficient

//always set this as 1 on ODROID
#define DVFS_EN 1 //1:change dvfs, 1:don't change dvfs (e.g., not running on ODROID)

//ONLINE related
#define ONLINE_EN 0 //0:off-line training, 1:on-line training
#define TYPE_PREDICT 0 //add selected features and return predicted time
#define TYPE_SOLVE 1 //add actual exec time and do optimization at on-line

//automatically set by platforms/architecture
#if DVFS_EN //ODROID
#define MAX_FREQ ((CORE)?(2000000):(1400000))
#define MAX_FREQ_BIG (2000000)
#define MAX_FREQ_LITTLE (1400000)
#define MIN_FREQ (200000)
#else //x86 laptop
#define MAX_FREQ ((CORE)?(2534000):(2534000))
#define MAX_FREQ_BIG (2534000)
#define MAX_FREQ_LITTLE (2535000)
#define MIN_FREQ (1199000)
#endif

#define ARCH_ARM 1 //ARM ODROID
#define ARCH_X86 0 //x86-laptop

#define _pocketsphinx_ 0
#define _stringsearch_ 0
#define _sha_preread_ 0
#define _rijndael_preread_ 0
#define _xpilot_slice_ 1
#define _2048_slice_ 0
#define _curseofwar_slice_sdl_ 0
#define _curseofwar_slice_ 0
#define _uzbl_ 0
#define _ldecode_ 0

//below benchmarks use file "times.txt" to print log 
#define F_PRINT ((_pocketsphinx_ || _2048_slice_ \
                  || _curseofwar_slice_ || _ldecode_)?(1):(0))

#if CORE //big
    #if _pocketsphinx_
        //max_exec * sweep / 100
        #define DEADLINE_TIME (double)((4000000*SWEEP)/100) 
    #else
        #define DEADLINE_TIME (double)((50000*SWEEP)/100)
    #endif
#else //LITTLE
    #if _pocketsphinx_
        #define DEADLINE_TIME (double)((4000000*SWEEP)/100)
    #else
        #define DEADLINE_TIME (double)((50000*SWEEP)/100)
    #endif
#endif

//Macro for common code blocks
#define _INIT_() \
  double exec_time = 0;\
  static int jump = 0;\
  if(check_define()==ERROR_DEFINE){\
      printf("%s", "DEFINE ERROR!!\n");\
      return ERROR_DEFINE;\
  }\
  llsp_t *solver = llsp_new(N_FEATURE + 1);
  /*  double exec_time = 0;
  static int jump = 0;
  if(check_define()==ERROR_DEFINE){
      printf("%s", "DEFINE ERROR!!\n");
      return ERROR_DEFINE;
  }
  llsp_t *solver = llsp_new(N_FEATURE + 1);*/
#define _DEFINE_TIME_() \
  exec_time = exec_timing();\
  int cur_freq = print_freq(); \
  int delay_time = 0;\
  int actual_delay_time = 0;\
  int additional_dvfs_times = 0;\
  int update_time = 0;
  /*exec_time = exec_timing();
  int cur_freq = print_freq(); 
  int delay_time = 0;
  int actual_delay_time = 0;
  int additional_dvfs_times = 0;
  int update_time = 0;*/
#define _DELAY_() \
  if(DELAY_EN && jump == 0 && ((delay_time = DEADLINE_TIME - exec_time \
          - slice_time - dvfs_time - update_time \
          - additional_dvfs_times) > 0)){\
    start_timing();\
    sleep_in_delay(delay_time, cur_freq);\
    end_timing();\
    actual_delay_time = exec_timing();\
  }else\
    delay_time = 0;\
  moment_timing_print(2); \
  print_delay_time(delay_time, actual_delay_time);\
  print_exec_time(exec_time);\
  print_total_time(exec_time + slice_time + dvfs_time + actual_delay_time);\
  print_update_time(update_time);
  /*if(DELAY_EN && jump == 0 && ((delay_time = DEADLINE_TIME - exec_time 
          - slice_time - dvfs_time - update_time 
          - additional_dvfs_times) > 0)){
    start_timing();
    sleep_in_delay(delay_time, cur_freq);
    end_timing();
    actual_delay_time = exec_timing();
  }else
    delay_time = 0;
  moment_timing_print(2); //moment_end
  print_delay_time(delay_time, actual_delay_time);
  print_exec_time(exec_time);
  print_total_time(exec_time + slice_time + dvfs_time + actual_delay_time);
  print_update_time(update_time);*/
#define _PRINT_INFO_() \
  if(HETERO_EN){\
    print_predicted_time(predicted_exec_time.big);\
    print_predicted_time(predicted_exec_time.little);\
  }else{\
    if(CORE)\
      print_predicted_time(predicted_exec_time.big);\
    else\
      print_predicted_time(predicted_exec_time.little);\
  }\
  /*if(HETERO_EN){
    print_predicted_time(predicted_exec_time.big);
    print_predicted_time(predicted_exec_time.little);
  }else{
    if(CORE)
      print_predicted_time(predicted_exec_time.big);
    else
      print_predicted_time(predicted_exec_time.little);
  }*/
//Depends on benchmarks
#if _sha_preread_
#define N_FEATURE 23
#define _SLICE_() sha_stream_slice(&sha_info, file_buffer, flen, solver)
#define SCALE (double)1000
#elif _rijndael_preread_
#define N_FEATURE 23
#define _SLICE_() encfile_slice(fout, ctx, argv[argv_i + 1], file_buffer, flen, solver)
#define SCALE (double)1000
#elif _stringsearch_
#define N_FEATURE 4
#define _SLICE_() slice(search_strings[i], solver);
#define SCALE (double)1
#elif _2048_slice_
#define N_FEATURE 95
#define _SLICE_() main_loop_slice(c, board, new_s, solver);
#define SCALE (double)1
#elif _curseofwar_slice_sdl_
#define N_FEATURE 14
#define _SLICE_() run_loop_slice(st, ui, screen, tileset, typeface, uisurf, tile_variant, pop_variant, k, solver);
#define SCALE (double)1
#elif _pocketsphinx_
#define N_FEATURE 11
#define _SLICE_() ps_process_raw_slice(ps, data, total, FALSE, TRUE);
#define SCALE (double)1
#elif _xpilot_slice_
#define N_FEATURE 250
#define _SLICE_() Main_loop_slice(solver);
#define SCALE (double)1
#else
#define N_FEATURE 4
#define SCALE (double)1
#endif

#define N_ERROR (10)
#define N_STABLE (4)
#define N_EVENT (3)
//#define N_STABLE N_FEATURE

/* codes from https://github.com/TUD-OS/ATLAS */
#define REMOVE_FACTOR (0.0)
#define COLUMN_CONTRIBUTION 1.1
typedef struct llsp_s llsp_t;
llsp_t *llsp_new(size_t count);
void llsp_add(llsp_t *restrict llsp, const double *restrict metrics, 
    double target, double remove_factor);
const double *llsp_solve(llsp_t *restrict llsp);
double llsp_predict(llsp_t *restrict llsp, const double *restrict metrics);
void llsp_dispose(llsp_t *restrict llsp);

/* float values below this are considered to be 0 */
#define EPSILON 1E-10

struct matrix {
	double **matrix;         // pointers to matrix data, indexed columns first, rows second
	size_t   columns;        // column count
};

struct llsp_s {
	size_t        metrics;   // metrics count
	double       *data;      // pointer to the malloc'ed data block, matrix is transposed
	struct matrix full;      // pointers to the matrix in its original form with all columns
	struct matrix sort;      // matrix with to-be-dropped columns shuffled to the right
	struct matrix good;      // reduced matrix with low-contribution columns dropped
	double        last_measured;
	double        result[];  // the resulting coefficients
};

static void givens_fixup(struct matrix m, size_t row, size_t column);
static void stabilize(struct matrix *sort, struct matrix *good);
static void trisolve(struct matrix m);




struct slice_return{
  double big;
  double little;
};

struct timeval start, end, moment;
struct timeval start_local, end_local;

void start_timing_local();
void end_timing_local();
double exec_timing_local();

double slice_time=0;
double dvfs_time=0;

FILE *fp_max_freq; //File pointer scaling_max_freq
FILE *fp_max_freq_big; //File pointer scaling_max_freq for big core
FILE *fp_max_freq_little; //File pointer scaling_max_freq for little core

void print_freq_power(int f_new_big, int f_new_little, float power_big, float power_little);
void print_current_core(int current_core, int big_little_cnt);
void print_est_time(int T_est_big, int T_est_little);

//////////////////////////////////////////////////////////////////////
//dvfs[i][j] -> dvfs_time from (i+2)*100 Mhz to (j+2)*100 Mhz
//////////////////////////////////////////////////////////////////////
#if CORE
  extern int dvfs_table[19][19];
#else
  extern int dvfs_table[13][13];
#endif
extern int dvfs_table_big[19][19];
extern int dvfs_table_little[13][13];

//////////////////////////////////////////////////////////////////////
//pro-active DVFS
//////////////////////////////////////////////////////////////////////
#if CORE //big
  #if _sha_preread_
    extern int predicted_times[99];
  #endif
  #if _xpilot_slice_
    extern int predicted_times[1001];
  #endif
  #if _stringsearch_
    extern int predicted_times[1332];
  #endif
  #if _2048_slice_
    extern int predicted_times[165];
  #endif
  #if _curseofwar_slice_sdl_
    extern int predicted_times[1002];
  #endif
  #if _curseofwar_slice_
    extern int predicted_times[1002];
  #endif
  #if _ldecode_
    extern int predicted_times[3000];
  #endif
  #if _pocketsphinx_
    extern int predicted_times[100];
  #endif
  #if _uzbl_
    extern int predicted_times[1];
  #endif
  #if _rijndael_preread_
    extern int predicted_times[99];
  #endif
#else //little
  #if _sha_preread_
    extern int predicted_times[99];
  #endif
  #if _xpilot_slice_
    extern int predicted_times[1001];
  #endif
  #if _stringsearch_
    extern int predicted_times[1332];
  #endif
  #if _2048_slice_
    extern int predicted_times[165];
  #endif
  #if _curseofwar_slice_sdl_
    extern int predicted_times[1002];
  #endif
  #if _curseofwar_slice_
    extern int predicted_times[1002];
  #endif
  #if _ldecode_
    extern int predicted_times[3000];
  #endif
  #if _pocketsphinx_
    extern int predicted_times[100];
  #endif
  #if _uzbl_
    extern int predicted_times[1];
  #endif
  #if _rijndael_preread_
    extern int predicted_times[99];
  #endif
#endif

//////////////////////////////////////////////////////////////////////
//predicted_times_big & predicted_times_little define
//////////////////////////////////////////////////////////////////////
#if _sha_preread_
  extern int predicted_times_big[99];
  extern int predicted_times_little[99];
#endif
#if _xpilot_slice_
  extern int predicted_times_big[1001];
  extern int predicted_times_little[1001];
#endif
#if _stringsearch_
  extern int predicted_times_big[1332];
  extern int predicted_times_little[1332];
#endif
#if _2048_slice_
  extern int predicted_times_big[165];
  extern int predicted_times_little[165];
#endif
#if _curseofwar_slice_sdl_
  extern int predicted_times_big[1002];
  extern int predicted_times_little[1002];
#endif
#if _curseofwar_slice_
  extern int predicted_times_big[1002];
  extern int predicted_times_little[1002];
#endif
#if _ldecode_
  extern int predicted_times_big[3000];
  extern int predicted_times_little[3000];
#endif
#if _pocketsphinx_
  extern int predicted_times_big[100];
  extern int predicted_times_little[100];
#endif
#if _uzbl_
  extern int predicted_times_big[1];
  extern int predicted_times_little[1];
#endif
#if _rijndael_preread_
  extern int predicted_times_big[99];
  extern int predicted_times_little[99];
#endif

//////////////////////////////////////////////////////////////////////
//power array
//////////////////////////////////////////////////////////////////////
extern float power_big[19]; 
extern float power_little[13];

int check_define(void){
    int flag_cnt=0;
    int bench_cnt=0;
	
    if(GET_PREDICT)     {flag_cnt++;}
    if(GET_OVERHEAD)    {flag_cnt++;}
    if(GET_DEADLINE)    {flag_cnt++;}
    if(PREDICT_EN)      {flag_cnt++;}
    if(CVX_EN)          {flag_cnt++;}
    if(OVERHEAD_EN)     {flag_cnt++;}
    if(ORACLE_EN)       {flag_cnt++;}
    if(PID_EN)          {flag_cnt++;}

    if(_pocketsphinx_)      {bench_cnt++;}
    if(_stringsearch_)      {bench_cnt++;}
    if(_sha_preread_)       {bench_cnt++;}
    if(_rijndael_preread_)  {bench_cnt++;}
    if(_xpilot_slice_)      {bench_cnt++;}
    if(_2048_slice_)        {bench_cnt++;}
    if(_curseofwar_slice_sdl_)  {bench_cnt++;}
    if(_curseofwar_slice_)  {bench_cnt++;}
    if(_uzbl_)              {bench_cnt++;}
    if(_ldecode_)              {bench_cnt++;}

    if( (flag_cnt==2 && PREDICT_EN==1 && OVERHEAD_EN==1)
        || (flag_cnt==2 && CVX_EN==1 && PREDICT_EN==1)
        || (flag_cnt==3 && CVX_EN==1 && PREDICT_EN==1 && OVERHEAD_EN==1)
        || (flag_cnt==0 && PREDICT_EN==0)
        || flag_cnt==1 
        || bench_cnt==1 )
        return 0;
    else
        return ERROR_DEFINE;
}

void fopen_all(void){
#if ARCH_ARM
  #if CORE //big
  if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq", "w"))){
    printf("(big) ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return;
  }
  #else //little
  if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "w"))){
    printf("(LITTLE) ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return;
  }
  #endif
  if(NULL == (fp_max_freq_big = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq", "w"))){
    printf("(big) ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return;
  }
  if(NULL == (fp_max_freq_little = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "w"))){
    printf("(LITTLE) ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return;
  }
#elif ARCH_X86
  if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq", "w"))){
    printf("(LITTLE) ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return;
  }
  if(NULL == (fp_max_freq_little = fopen("/sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq", "w"))){
    printf("(LITTLE) ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return;
  }
#endif
}

void fclose_all(void){
  fclose(fp_max_freq);
#if ARCH_ARM
  fclose(fp_max_freq_big);
#endif
  fclose(fp_max_freq_little);
  return;
}

double set_freq_to_specific(int khz){
	start_timing_local();
  fprintf(fp_max_freq, "%d", khz);
  fflush(fp_max_freq);
  end_timing_local();
  return exec_timing_local();
}

void set_freq_to_specific_no_timing(int khz){
  fprintf(fp_max_freq, "%d", khz);
  fflush(fp_max_freq);
  return;
}

void sleep_in_delay(double delay_time, int cur_freq){
#if !IDLE_EN
  usleep((int)delay_time);
#else
  //Set to lowest freq
  double set_freq_delay = set_freq_to_specific(MIN_FREQ);
  //Delay at lowest freq
  if(delay_time - set_freq_delay - dvfs_table[cur_freq/100000-2][MIN_FREQ/100000-2] > 0)
    usleep(int(delay_time - set_freq_delay -
          (double)dvfs_table[cur_freq/100000-2][MIN_FREQ/100000-2]));
  //Back to previous freq
  set_freq_to_specific_no_timing(cur_freq);
#endif
}

void set_freq(double predicted_exec_time, double slice_time, 
    double deadline_time, double avg_dvfs_time){
  int predicted_freq = MAX_FREQ;
#if ARCH_ARM
  static int previous_freq = MAX_FREQ;
  double job_exec_time;
  for(predicted_freq = 200000; predicted_freq < MAX_FREQ+1; predicted_freq += 100000){
    job_exec_time = MARGIN * predicted_exec_time * (double)MAX_FREQ / (double)predicted_freq;
  #if OVERHEAD_EN // with dvfs + slice overhead
    if(job_exec_time + (double)dvfs_table[previous_freq/100000-2][predicted_freq/100000-2] + slice_time < deadline_time)
        break;
  #elif SLICE_OVERHEAD_ONLY_EN // with slice overhead only
    if(job_exec_time + slice_time < deadline_time)
        break;
  #else // without dvfs + slice and oracle
    if(job_exec_time < deadline_time)
        break;
  #endif
  }     
#elif ARCH_X86	
  //calculate predicted freq and round up by adding 99999
  predicted_freq = (int)(MARGIN * predicted_exec_time * MAX_FREQ /
      (deadline_time - slice_time - avg_dvfs_time) + 99999);
#endif
  //if less then 200000, just set it minimum (200000)
  predicted_freq = (predicted_freq < MIN_FREQ || predicted_exec_time <= 1)?(MIN_FREQ):(predicted_freq);
#if ARCH_ARM
  //remember current frequency to use later
  previous_freq = predicted_freq;
#endif
  //set maximum frequency, because performance governor always use maximum freq.
  fprintf(fp_max_freq, "%d", predicted_freq);
  fflush(fp_max_freq);
  return;
}

void set_freq_hetero(int T_est_big, int T_est_little, int slice_time, int d, int avg_dvfs_time, int pid){
	int f_max_big = MAX_FREQ_BIG/1000;//khz->mhz
	int f_max_little = MAX_FREQ_LITTLE/1000;//khz->mhz
	int f_new = MAX_FREQ/1000;
  int final_freq = MAX_FREQ/1000;
	int f_new_big = MAX_FREQ_BIG/1000;
	int f_new_little = MAX_FREQ_LITTLE/1000;
	int T_sum_big = 0;
	int T_sum_little = 0;
	static int f_previous_big = MAX_FREQ_BIG/1000;
	static int f_previous_little = MAX_FREQ_LITTLE/1000;
  static int big_cnt = 0;
  static int little_cnt = 0;
  static int current_core = CORE; //0: little, 1: big
  cpu_set_t set;
#if DEBUG_EN
  char cmd[100];
#endif
    
    for(f_new_big = 200; f_new_big < f_max_big+1; f_new_big += 100){
        T_sum_big = MARGIN * T_est_big * f_max_big / f_new_big;
    #if OVERHEAD_EN // with dvfs + slice overhead
        if(T_sum_big + dvfs_table_big[f_previous_big/100-2][f_new_big/100-2] + slice_time < d)
            break;
    #elif SLICE_OVERHEAD_ONLY_EN // with slice overhead only
        if(T_sum_big + slice_time < d)
            break;
    #else // without dvfs + slice and oracle
        if(T_sum_big < d)
            break;
    #endif
    }      
    for(f_new_little = 200; f_new_little < f_max_little+1; f_new_little += 100){
        T_sum_little = MARGIN * T_est_little * f_max_little / f_new_little;
        //printf("sum time %d @%d\n", T_sum_little, f_previous_little);
    #if OVERHEAD_EN // with dvfs + slice overhead
        if(T_sum_little + dvfs_table_little[f_previous_little/100-2][f_new_little/100-2] + slice_time < d)
            break;
    #elif SLICE_OVERHEAD_ONLY_EN // with slice overhead only
        if(T_sum_little + slice_time < d)
            break;
    #else // without dvfs + slice and oracle
        if(T_sum_little < d)
            break;
    #endif
    }     
    //printf("est time %d @%d\n", T_est_little, f_new_little);
    //round up to be conservative (ex: 123 Mhz -> 200 Mhz, 987 Mhz -> 1000Mhz)
    f_new_big = ((int)((f_new_big + 99) / 100)) * 100;
    f_new_little = ((int)((f_new_little + 99) / 100)) * 100;
    f_new_big = (f_new_big < 200)?(200):(f_new_big);
    f_new_little = (f_new_little < 200)?(200):(f_new_little);
    //printf("f_new big:little (Mhz) = %d:%d\n", f_new_big, f_new_little);
    //printf("power big:little (W) = %f:%f\n", power_big[f_new_big/100-2], power_little[f_new_little/100-2]);
    //Select cores and use taskset command to migrate process if it was on different core
    //f_new = f_new_little or f_new_big? let's compare power, because times will be same
    //1. If big power is smaller or little freq is over max freq, select big core
    //2. If little power is smaller, select little core
    //3. If same, no change
    
    //debug
    //#if DEBUG_EN
//    print_freq_power(f_new_big, f_new_little, power_big[f_new_big/100-2], power_little[f_new_little/100-2]);
    //#endif
 
    if(power_big[f_new_big/100-2] < power_little[f_new_little/100-2]
            || f_new_little > 1400){  
        f_new = f_new_big;
        //printf("big %d times\n", ++big_cnt);
        if(!current_core){//if it was little core
            CPU_ZERO( &set );
            CPU_SET( 4, &set );
            CPU_SET( 5, &set );
            CPU_SET( 6, &set );
            CPU_SET( 7, &set );
            if (sched_setaffinity( pid, sizeof( cpu_set_t ), &set ))
            {
                perror( "sched_setaffinity" );
                exit(2);
            }
            #if DEBUG_EN
            sprintf(cmd, "taskset -p %d", pid);
            fflush(stdout);
            system(cmd);
            #endif
        }
        current_core = 1;
        print_current_core(current_core, ++big_cnt);
    }else if(power_big[f_new_big/100-2] > power_little[f_new_little/100-2]){
        f_new = f_new_little;
        //printf("little %d times\n", ++little_cnt);
        if(current_core){//if it was big core
            CPU_ZERO( &set );
            CPU_SET( 0, &set );
            CPU_SET( 1, &set );
            CPU_SET( 2, &set );
            CPU_SET( 3, &set );
            if (sched_setaffinity( pid, sizeof( cpu_set_t ), &set ))
            {
                perror( "sched_setaffinity" );
                exit(2);
            }
            #if DEBUG_EN
            sprintf(cmd, "taskset -p %d", pid);
            fflush(stdout);
            system(cmd);
            #endif
        }
        current_core = 0;
        print_current_core(current_core, ++little_cnt);
    }else{
        if(current_core)
            print_current_core(current_core, ++big_cnt);
        else
            print_current_core(current_core, ++little_cnt);
    }
    //if less then 200 Mhz, just set it minimum (200)
    //f_new = (f_new < 200)?(200):(f_new);
    //save previous freq
	f_previous_big = f_new_big;
	f_previous_little = f_new_little;
    //previous freq should not exceed max_freq
	f_previous_big = (f_previous_big > MAX_FREQ_BIG/1000)?(MAX_FREQ_BIG/1000):(f_previous_big);
	f_previous_little = (f_previous_little > MAX_FREQ_LITTLE/1000)?(MAX_FREQ_LITTLE/1000):(f_previous_little);
	//mhz->khz
	final_freq = f_new*1000;
    //set maximum frequency, because performance governor always use maximum freq.

    if(current_core){
        fprintf(fp_max_freq_big, "%d", final_freq);
        fflush(fp_max_freq_big);
//        fprintf(fp_max_freq_little, "%d", MAX_FREQ_LITTLE);
//        fflush(fp_max_freq_little);
    }else{
        fprintf(fp_max_freq_little, "%d", final_freq);
        fflush(fp_max_freq_little);
//        fprintf(fp_max_freq_big, "%d", MAX_FREQ_BIG);
//        fflush(fp_max_freq_big);
    }
    return;
}
/*
	int job : job number
	int d : deadline
    int w : window size
    return jump
*/
int set_freq_multiple(int job, int d){
	int w=WINDOW_SIZE;
	int i, j, k, brk;
	int f_max = MAX_FREQ/1000;//khz->mhz
	int f_new, final_freq;
	int T_sum = 0;
	int T_est[WINDOW_SIZE];
	int size = sizeof(predicted_times)/sizeof(predicted_times[0]);
	static int jump = 0; //if jump is 0, set new freq
	//static int group = 0; //how many jobs are grouped
	static int f_previous = MAX_FREQ;

	if(jump == 0){
		//estimate time for multiple jobs (from current to current + W)
		//printf("T_est : ");
		for(i=0; i<w; i++){
			if(job+i < size)
				T_est[i] = predicted_times[job+i];
			else
				T_est[i] = -1;
			//printf("%d, ", T_est[i]);
			//#if DEBUG_EN
			//print_est_time(T_est[i], -99);
			//#endif
		}
		//print_enter();
		for(i=w; i>0; i--){
			//1. Sum total time
			T_sum = 0;
			for(j=0; j<i; j++)
				T_sum += MARGIN * T_est[j];
            #if OVERHEAD_EN //just add 1000us for simplicity
            T_sum += 1000;
            #endif
			//printf("T_sum = %d\t", T_sum);
			//2. Calculate new frequecy
			f_new = (f_max * T_sum)/(i*d);
            //round up to be conservative (ex: 123 Mhz -> 200 Mhz, 987 Mhz -> 1000Mhz)
	        f_new = ((int)((f_new + 99) / 100)) * 100;
			//printf("f_new (Mhz) = %d\n", f_new);
            //#if DEBUG_EN
            //print_freq_power(f_new, -99, -99, -99);
            //#endif

			//3. Check if all deadlines will be met
			brk = 0;
			
			for(j=1; j<i; j++){
				T_sum = 0;
				for(k=0; k<j; k++)
					T_sum += T_est[k];
				if((T_sum * f_max) <= (j * d * f_new))
					brk++;
			}
			if(brk == i-1){
				jump = i-1;
                //group = i;
				//printf("group of %d\n", group);
				break;
			}
		}	
	}else{
		f_new = f_previous; 
		//printf("f_new (Mhz) = %d\n", f_new);
		jump--;
		//printf("group of %d\n", group);
	}
	//printf("jump = %d\n", jump);

    //if less then 200 Mhz, just set it minimum (200)
    f_new = (f_new < 200)?(200):(f_new);
    //save previous freq
	f_previous = f_new;
	//mhz->khz
	final_freq = f_new*1000;
    //set maximum frequency, because performance governor always use maximum freq.
    fprintf(fp_max_freq, "%d", final_freq);
    fflush(fp_max_freq);
    return jump;
}

int set_freq_multiple_hetero(int job, int d, int pid){
	int w=WINDOW_SIZE;
	int i, j, k;
	int brk_big, brk_little;
	int f_max_big = MAX_FREQ_BIG/1000;//khz->mhz
	int f_max_little = MAX_FREQ_LITTLE/1000;//khz->mhz
	int f_new = MAX_FREQ/1000;
  int final_freq = MAX_FREQ/1000;
	int f_new_big;
	int f_new_little;
	int T_sum_big = 0;
	int T_sum_little = 0;
	int T_est_big[WINDOW_SIZE];
	int T_est_little[WINDOW_SIZE];
	int size_big = sizeof(predicted_times_big)/sizeof(predicted_times_big[0]);
	//int size_little = sizeof(predicted_times_little)/sizeof(predicted_times_little[0]);
	static int jump = 0; //if jump is 0, set new freq
	//static int group = 0; //how many jobs are grouped
	static int f_previous = MAX_FREQ;
  static int big_cnt = 0;
  static int little_cnt = 0;
  static int current_core = CORE; //0: little, 1: big
  cpu_set_t set;
#if DEBUG_EN
  char cmd[100];
#endif

	if(jump == 0){
		//estimate time for multiple jobs (from current to current + W)
		for(i=0; i<w; i++){
			if(job+i < size_big){
				T_est_big[i] = predicted_times_big[job+i];
				T_est_little[i] = predicted_times_little[job+i];
			}
			else{
				T_est_big[i] = -1;
				T_est_little[i] = -1;
			}
			//#if DEBUG_EN
			//print_est_time(T_est_big[i], T_est_little[i]);
			//#endif
		}
		//print_enter();
		for(i=w; i>0; i--){
			//1. Sum total time
			T_sum_big = 0;
			T_sum_little = 0;
			for(j=0; j<i; j++){
				T_sum_big += MARGIN * T_est_big[j];
				T_sum_little += MARGIN * T_est_little[j];
            }
            #if OVERHEAD_EN //just add 1000us for simplicity
            T_sum_big += 1000;
            T_sum_little += 1000;
            #endif
			//printf("T_sum big:little = %d:%d\t", T_sum_big, T_sum_little);
			//2. Calculate new frequecy
			f_new_big = (f_max_big * T_sum_big)/(i*d);
			f_new_little = (f_max_little * T_sum_little)/(i*d);
            //round up to be conservative (ex: 123 Mhz -> 200 Mhz, 987 Mhz -> 1000Mhz)
	        f_new_big = ((int)((f_new_big + 99) / 100)) * 100;
	        f_new_little = ((int)((f_new_little + 99) / 100)) * 100;
            f_new_big = (f_new_big < 200)?(200):(f_new_big);
            f_new_little = (f_new_little < 200)?(200):(f_new_little);
            
            //#if DEBUG_EN
            //print_freq_power(f_new_big, f_new_little, power_big[f_new_big/100-2], power_little[f_new_little/100-2]);
            //#endif

			//3. Check if all deadlines will be met
			brk_big = 0;
			brk_little = 0;
			
			for(j=1; j<i; j++){
				T_sum_big = 0;
				T_sum_little = 0;
				for(k=0; k<j; k++){
					T_sum_big += T_est_big[k];
					T_sum_little += T_est_little[k];
                }
				if((T_sum_big * f_max_big) <= (j * d * f_new_big))
					brk_big++;
				if((T_sum_little * f_max_little) <= (j * d * f_new_little))
					brk_little++;
			}
			if(brk_big == i-1 && brk_little == i-1){
				jump = i-1;
                //group = i;
				//printf("group of %d\n", group);
				break;
			}
		}	
        //Select cores and use taskset command to migrate process if it was on different core
        //f_new = f_new_little or f_new_big? let's compare power, because times will be same
        //1. If big power is smaller or little freq is over max freq, select big core
        //2. If little power is smaller, select little core
        //3. If same, no change
        if(power_big[f_new_big/100-2] < power_little[f_new_little/100-2]
             || f_new_little > 1400){  
            f_new = f_new_big;
            //printf("big %d times\n", ++big_cnt);
            if(!current_core){//if it was little core
                CPU_ZERO( &set );
                CPU_SET( 4, &set );
                CPU_SET( 5, &set );
                CPU_SET( 6, &set );
                CPU_SET( 7, &set );
                if (sched_setaffinity( pid, sizeof( cpu_set_t ), &set ))
                {
                    perror( "sched_setaffinity" );
                    exit(2);
                }
                #if DEBUG_EN
                sprintf(cmd, "taskset -p %d", pid);
                fflush(stdout);
                system(cmd);
                #endif
            }
            current_core = 1;
            print_current_core(current_core, ++big_cnt);
        }else if(power_big[f_new_big/100-2] > power_little[f_new_little/100-2]){
            f_new = f_new_little;
            //printf("little %d times\n", ++little_cnt);
            if(current_core){//if it was big core
                CPU_ZERO( &set );
                CPU_SET( 0, &set );
                CPU_SET( 1, &set );
                CPU_SET( 2, &set );
                CPU_SET( 3, &set );
                if (sched_setaffinity( pid, sizeof( cpu_set_t ), &set ))
                {
                    perror( "sched_setaffinity" );
                    exit(2);
                }
                #if DEBUG_EN
                sprintf(cmd, "taskset -p %d", pid);
                fflush(stdout);
                system(cmd);
                #endif
            }
            current_core = 0;
            print_current_core(current_core, ++little_cnt);
        }else{
            if(current_core)
                print_current_core(current_core, ++big_cnt);
            else
                print_current_core(current_core, ++little_cnt);
        }
           
	}else{
		f_new = f_previous; 
		//printf("f_new (Mhz) = %d\n", f_new);
		jump--;
		//printf("group of %d\n", group);
        //if(current_core)
        //    printf("big %d times\n", ++big_cnt);
        //else
        //    printf("little %d times\n", ++little_cnt);
        if(current_core)
            print_current_core(current_core, ++big_cnt);
        else
            print_current_core(current_core, ++little_cnt);
	}
	//printf("jump = %d\n", jump);

    //if less then 200 Mhz, just set it minimum (200)
    //f_new = (f_new < 200)?(200):(f_new);
    //save previous freq
	f_previous = f_new;
	//mhz->khz
	final_freq = f_new*1000;
    //set maximum frequency, because performance governor always use maximum freq.

    if(current_core){
        fprintf(fp_max_freq_big, "%d", final_freq);
        fflush(fp_max_freq_big);
    }else{
        fprintf(fp_max_freq_little, "%d", final_freq);
        fflush(fp_max_freq_little);
    }
    return jump;
}


#if !F_PRINT //just use printf
int print_freq(void){
  FILE *fp_freq; //File pointer of freq of A7 (LITTLE) core or A15 (big) core power sensor file
  int khz; //Value (khz) at start point.
#if ARCH_ARM
#if CORE
  if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq", "r"))){
    printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return -1;
  }
  if(fscanf(fp_freq, "%d", &khz)<0)
    return -1;
  printf("little core freq : %dkhz\n", khz);  
  fclose(fp_freq);
  return khz;
#else
  if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r"))){
    printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return -1;
  }
  if(fscanf(fp_freq, "%d", &khz)<0)
    return -1;
  printf("little core freq : %dkhz\n", khz);  
  fclose(fp_freq);
  return khz;
#endif
#elif ARCH_X86
  if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq", "r"))){
    printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return -1;
  }
  if(fscanf(fp_freq, "%d", &khz)<0)
    return -1;
  printf("little core freq : %dkhz\n", khz);  
  fclose(fp_freq);
  return khz;
#endif
}
#elif F_PRINT //some benchmarks use file "times.txt" to print log 
int print_freq(void){
  FILE *fp_freq; //File pointer of freq of A7 (LITTLE) core or A15 (big) core power sensor file
  int khz; //Value (khz) at start point.
  FILE *time_file;
  time_file = fopen("times.txt", "a");
#if ARCH_ARM
#if CORE
  if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq", "r"))){
    printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return -1;
  }
  if(fscanf(fp_freq, "%d", &khz)<0)
    return -1;
  fprintf(time_file, "little core freq : %dkhz\n", khz);  
  fclose(fp_freq);
  fclose(time_file); 
  return khz;
#else
  if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r"))){
    printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return -1;
  }
  if(fscanf(fp_freq, "%d", &khz)<0)
    return -1;
  fprintf(time_file, "little core freq : %dkhz\n", khz);  
  fclose(fp_freq);
  fclose(time_file); 
  return khz;
#endif
#elif ARCH_X86
  if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq", "r"))){
    printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
    return -1;
  }
  if(fscanf(fp_freq, "%d", &khz)<0)
    return -1;
  fprintf(time_file, "little core freq : %dkhz\n", khz);  
  fclose(fp_freq);
  fclose(time_file); 
  return khz;
#endif
}


#endif



/* llsp related codes from https://github.com/TUD-OS/ATLAS */
llsp_t *llsp_new(size_t count)
{
	llsp_t *llsp;
	
	if (count < 1) return NULL;
	
	size_t llsp_size = sizeof(llsp_t) + count * sizeof(double);  // extra room for coefficients
	llsp = malloc(llsp_size);
	if (!llsp) return NULL;
	memset(llsp, 0, llsp_size);
	
	llsp->metrics = count;
	llsp->full.columns = count + 1;
	llsp->sort.columns = count + 1;
	
	return llsp;
}

void llsp_add(llsp_t *restrict llsp, const double *restrict metrics,
    double target, double remove_factor)
{
	const size_t column_count = llsp->full.columns;
	const size_t row_count = llsp->full.columns + 1;  // extra row for shifting down and trisolve
	const size_t column_size = row_count * sizeof(double);
	const size_t data_size = column_count * row_count * sizeof(double);
	const size_t matrix_size = column_count * sizeof(double *);
	const size_t index_last = column_count - 1;
	
	if (!llsp->data) {
		llsp->data        = malloc(data_size);
		llsp->full.matrix = malloc(matrix_size);
		llsp->sort.matrix = malloc(matrix_size);
		llsp->good.matrix = malloc(matrix_size);
		if (!llsp->data || !llsp->full.matrix || !llsp->sort.matrix || !llsp->good.matrix)
			abort();
		
		for (size_t column = 0; column < llsp->full.columns; column++)
			llsp->full.matrix[column] =
			llsp->sort.matrix[column] = llsp->data + column * row_count;
		
		/* we need an extra column for the column dropping scan */
		llsp->good.matrix[index_last] = malloc(column_size);
		if (!llsp->good.matrix[index_last]) abort();
		
		memset(llsp->data, 0, data_size);
	}
	
	/* age out the past a little bit */
	for (size_t element = 0; element < row_count * column_count; element++)
		llsp->data[element] *= 1.0 - remove_factor;
	
	/* add new row to the top of the solving matrix */
	memmove(llsp->data + 1, llsp->data, data_size - sizeof(double));
	for (size_t column = 0; column < llsp->metrics; column++)
		llsp->full.matrix[column][0] = metrics[column];
	llsp->full.matrix[llsp->metrics][0] = target;
	
	/* givens fixup of the subdiagonal */
	for (size_t i = 0; i < llsp->sort.columns; i++)
		givens_fixup(llsp->sort, i + 1, i);
	
	llsp->last_measured = target;
}

const double *llsp_solve(llsp_t *restrict llsp)
{
	double *result = NULL;
	
	if (llsp->data) {
		stabilize(&llsp->sort, &llsp->good);
		trisolve(llsp->good);
		
		/* collect coefficients */
		size_t result_row = llsp->good.columns;
		for (size_t column = 0; column < llsp->metrics; column++)
			llsp->result[column] = llsp->full.matrix[column][result_row];
		result = llsp->result;
	}
	
	return result;
}

double llsp_predict(llsp_t *restrict llsp, const double *restrict metrics)
{
	/* calculate prediction by dot product */
	double result = 0.0;
	for (size_t i = 0; i < llsp->metrics; i++){
		result += llsp->result[i] * metrics[i];
  }
	
	if (result >= EPSILON)
		return result;
	else
		return llsp->last_measured;
}

void llsp_dispose(llsp_t *restrict llsp)
{
	const size_t index_last = llsp->good.columns - 1;
	
	free(llsp->good.matrix[index_last]);
	free(llsp->full.matrix);
	free(llsp->sort.matrix);
	free(llsp->good.matrix);
	free(llsp->data);
	free(llsp);
}

static void givens_fixup(struct matrix m, size_t row, size_t column)
{
	if (fabs(m.matrix[column][row]) < EPSILON) {  // alread zero
		m.matrix[column][row] = 0.0;  // reset to an actual zero for stability
		return;
	}
	
	const size_t i = row;
	const size_t j = column;
	const double a_ij = m.matrix[j][i];
	const double a_jj = m.matrix[j][j];
	const double rho = ((a_jj < 0.0) ? -1.0 : 1.0) * sqrt(a_jj * a_jj + a_ij * a_ij);
	const double c = a_jj / rho;
	const double s = a_ij / rho;
	
	for (size_t x = column; x < m.columns; x++) {
		if (x == column) {
			// the real calculation below should produce the same, but this is more stable
			m.matrix[x][i] = 0.0;
			m.matrix[x][j] = rho;
		} else {
			const double a_ix = m.matrix[x][i];
			const double a_jx = m.matrix[x][j];	
			m.matrix[x][i] = c * a_ix - s * a_jx;
			m.matrix[x][j] = s * a_ix + c * a_jx;
		}
		
		// reset to an actual zero for stability
		if (fabs(m.matrix[x][i]) < EPSILON)
			m.matrix[x][i] = 0.0;
		if (fabs(m.matrix[x][j]) < EPSILON)
			m.matrix[x][j] = 0.0;
	}
}

static void stabilize(struct matrix *sort, struct matrix *good)
{
	const size_t column_count = sort->columns;
	const size_t row_count = sort->columns + 1;  // extra row for shifting down and trisolve
	const size_t column_size = row_count * sizeof(double);
	const size_t index_last = column_count - 1;
	
	bool drop[column_count];
	double previous_residual = 0.0;
	
	good->columns = sort->columns;
	memcpy(good->matrix[index_last], sort->matrix[index_last], column_size);
	
	/* Drop columns from right to left and watch the residual error.
	 * We would actually copy the whole matrix, but when dropping from the right,
	 * Givens fixup always affects only the last column, so we hand just the
	 * last column through all possible positions. */
	for (size_t column = index_last; (ssize_t)column >= 0; column--) {
		good->matrix[column] = good->matrix[index_last];
		givens_fixup(*good, column + 1, column);
		
		double residual = fabs(good->matrix[column][column]);
		if (residual >= EPSILON && previous_residual >= EPSILON)
			drop[column] = (residual / previous_residual < COLUMN_CONTRIBUTION);
		else if (residual >= EPSILON && previous_residual < EPSILON)
			drop[column] = false;
		else
			drop[column] = true;
		
		previous_residual = residual;
		good->columns--;
	}
	/* The drop result for the last column is never used. The last column
	 * represents our target vector, so we must never drop it. */
	
	/* shuffle all to-be-dropped columns to the right */
	size_t keep_columns = index_last;  // number of columns to keep, starts with all
	for (size_t drop_column = index_last - 1; (ssize_t)drop_column >= 0; drop_column--) {
		if (!drop[drop_column]) continue;
		
		keep_columns--;
		
		if (drop_column < keep_columns) {  // column must move
			double *temp = sort->matrix[drop_column];
			memmove(&sort->matrix[drop_column], &sort->matrix[drop_column + 1],
					(keep_columns - drop_column) * sizeof(double *));
			sort->matrix[keep_columns] = temp;
			
			for (size_t column = drop_column; column < keep_columns; column++)
				givens_fixup(*sort, column + 1, column);
		}
	}
	
	/* setup good-column matrix */
	good->columns = sort->columns;
	memcpy(good->matrix, sort->matrix, keep_columns * sizeof(double *));  // non-drop columns
	memcpy(good->matrix[index_last], sort->matrix[index_last], column_size);   // copy last column
	
	/* Conceptually, we now drop the to-be-dropped columns from the right.
	 * Again, dropping the from the right only affects the residual error
	 * in the last column, so only it changes. Further, we no longer need
	 * the residual, so we can omit a proper Givens fixup and zero the
	 * residual instead.
	 * The resulting matrix has the same number of columns as the input,
	 * so the extra bottom-row used later by trisolve to store coefficients
	 * will be the actual bottom-row and not destroy triangularity.
	 * The resulting coeffients however will be the same as with an actual
	 * column-reduced matrix, because the diagonal elements for all
	 * dropped columns are zero. */
	for (size_t column = index_last; (ssize_t)column >= (ssize_t)keep_columns; column--) {
		good->matrix[column] = good->matrix[index_last];
		good->matrix[column][column] = 0.0;
	}
}

static void trisolve(struct matrix m)
{
	size_t result_row = m.columns;  // use extra row to solve the coefficients
	for (size_t column = 0; column < m.columns - 1; column++)
		m.matrix[column][result_row] = 0.0;
	
	for (size_t row = result_row - 2; (ssize_t)row >= 0; row--) {
		size_t column = row;
		
		if (fabs(m.matrix[column][row]) >= EPSILON) {
			column = m.columns - 1;
			
			double intermediate = m.matrix[column][row];
			for (column--; column > row; column--)
				intermediate -= m.matrix[column][result_row] * m.matrix[column][row];
			m.matrix[column][result_row] = intermediate / m.matrix[column][row];

			for (column--; (ssize_t)column >= 0; column--)
				// must be upper triangular matrix
				assert(m.matrix[column][row] == 0.0);
		} else
			m.matrix[column][row] = 0.0;  // reset to an actual zero for stability
	}
}

//////////////////////////////////////////////////////////////////////
// on-line training core function
// type == prediction : update execution time (y) with scaled freq
//                      return predicted time 
// type == update     : update loop counter (x)
//                      solve MLSR to find (betha)
// Check errro in consecutive jobs for the interference event
//////////////////////////////////////////////////////////////////////
int func_is_stable(double errors[N_ERROR], int n_stable){
  int avg_error = 0;
  for(int i = 0; i < n_stable ; i++){
    avg_error += fabs(errors[i]);
  }
  avg_error /= n_stable;
  if(avg_error > 10.0)
    return 0;
  else
    return 1;
}
int func_is_event(double errors[N_ERROR], int n_event){
  int is_event = 1;
  int avg_error = 0;
  //if any error in n_event is less than 10%, we count this as just outlier
  //when errors in n_event consecutive jobs, we count this as an event
  for(int i = 0; i < n_event ; i++){
    avg_error += fabs(errors[i]);
    if(errors[i] < 10.0)
      is_event = 0;
  }
  avg_error /= n_event;
  if(avg_error > 10.0)
    is_event = 1;

  return is_event;
}
double get_predicted_time(int type, llsp_t *restrict solver, int *loop_counter,
    int size, int actual_exec_time, int freq)
{ 
  int i;
  static double error = 100.0; //error = |actual-predicted|/actual*100
  static double errors[N_ERROR] = {0}; //save the past errors
  static int is_stable = 0; //indicator whether prediction is stable
  static double exec_time = 0; //predicted execution time
  static double metrics[N_FEATURE+1] = {0}; //For constant term, increase size by 1
  static double remove_factor = 0.0; //remove factor

  if(type == TYPE_PREDICT)//add selected features, return predicted time
  {
    //update params.xx, add 1 to leftmost column for constant term
    metrics[0] = (double)1/SCALE;
    for(i = 0; i < N_FEATURE ; i++)
      metrics[i+1] = (double)loop_counter[i]/SCALE;

    //get predicted time
    exec_time = llsp_predict(solver, metrics);

    //check error in previous job, and decide return value
    //if(fabs(error) > 10.0){//if |error| > 10%, return highest exec time
    if(!is_stable){//be conservative until prediction is stable
#if DELAY_EN
      return DEADLINE_TIME;
#else
      return exec_time;
#endif
    }
    //else//if |error| <= 10% (i.e. 90% accuracy), use predicted value
    else//as soon as it is stable, we can use predicted time
      return exec_time;
  }
  else if(type == TYPE_SOLVE)//add actual exec time, do optimization on-line
  {
    //update params.yy, we assume time is scaled by freq linearly
    double scaled_actual_exec_time = (double)actual_exec_time *
      ((double)freq/(double)MAX_FREQ);
    llsp_add(solver, metrics, scaled_actual_exec_time, remove_factor);
    
    //reset remove_factore as 0 
    remove_factor = 0.0;

    //solve with updated params.xx and params.yy
    (void)llsp_solve(solver);

    //calculate an error 
    error = (scaled_actual_exec_time - (double)exec_time)/scaled_actual_exec_time*100;

    //update errors array, keep newest one at the first index 
    for(int j = N_ERROR-1 ; j > 0; j--)
      errors[j] = errors[j-1];
    errors[0] = error;

    //for(int j=0; j<N_ERROR; j++)
    //  printf("%f, ", errors[j]);

    //check stability
    is_stable = func_is_stable(errors, N_STABLE);

    //While prediction is stable, if we find errors in consecutive jobs, we
    //give up old data by removing factor (if 0.9, decrease by 90%)
    if(is_stable && func_is_event(errors, N_EVENT)){
      printf("old data removed\n");
      remove_factor = 1.00;
    }

    return -1;//return dummy
  }else{
    perror( "unknown type (should be TYPE_UPDATE or TYPE_SOLVE)" );
    return -1;
  }
}

/*
 * PID-based prediction of execution time.
 */
#if CORE //big
    #if _sha_preread_
        #define PID_P 1.150000
        #define PID_I 1.200000
        #define PID_D 1.000000
    #endif
    #if _xpilot_slice_
        #define PID_P 1.300000
        #define PID_I 1.000000
        #define PID_D 1.100000
    #endif
    #if _stringsearch_
        #define PID_P 1.100000
        #define PID_I 1.100000
        #define PID_D 1.050000
    #endif
    #if _2048_slice_
        #define PID_P 1.100000
        #define PID_I 1.500000
        #define PID_D 1.000000
    #endif
    #if _curseofwar_slice_sdl_
        #define PID_P 1.050000
        #define PID_I 1.900000
        #define PID_D 1.000000
    #endif
    #if _curseofwar_slice_
        #define PID_P 1.050000
        #define PID_I 1.900000
        #define PID_D 1.000000
    #endif
    #if _ldecode_
        #define PID_P 0.900000
        #define PID_I 0.100000
        #define PID_D 0.000000
    #endif
    #if _pocketsphinx_
        #define PID_P 0.150000
        #define PID_I 0.400000
        #define PID_D 0.000000
    #endif
    #if _uzbl_
        #define PID_P 1.050000
        #define PID_I 1.900000
        #define PID_D 1.000000
    #endif
    #if _rijndael_preread_
        #define PID_P 1.050000
        #define PID_I 1.300000
        #define PID_D 1.000000
    #endif
#else //little
    #if _sha_preread_
        #define PID_P 0.100000
        #define PID_I 0.300000
        #define PID_D 0.100000
    #endif
    #if _xpilot_slice_
        #define PID_P 0.350000
        #define PID_I 0.000000
        #define PID_D 0.150000
    #endif
    #if _stringsearch_
        #define PID_P 0.450000
        #define PID_I 0.000000
        #define PID_D 0.000000
    #endif
    #if _2048_slice_
        #define PID_P 0.450000
        #define PID_I 0.400000
        #define PID_D 0.050000
    #endif
    #if _curseofwar_slice_sdl_
        #define PID_P 0.150000
        #define PID_I 1.600000
        #define PID_D 0.000000
    #endif
    #if _curseofwar_slice_
        #define PID_P 0.150000
        #define PID_I 1.600000
        #define PID_D 0.000000
    #endif
    #if _ldecode_
        #define PID_P 0.050000
        #define PID_I 0.100000
        #define PID_D 0.350000
    #endif
    #if _pocketsphinx_
        #define PID_P 0.250000
        #define PID_I 0.100000
        #define PID_D 0.100000
    #endif
    #if _uzbl_
        #define PID_P 0.350000
        #define PID_I 2.100000
        #define PID_D 0.050000
    #endif
    #if _rijndael_preread_
        #define PID_P 0.050000
        #define PID_I 0.100000
        #define PID_D 0.250000
    #endif
#endif

float pid_controller(int last_time) {
  // Define variables
  float d_error;
  static float error = 0; // Absolute error
  static float i_error = 0; // Integrative error
  static float predicted_time = 0; // Saved prediction time

  // Calculate errors
  // Derivative is new error minus old error 
  d_error = (last_time - predicted_time) - error; 
  // Update error
  error = last_time - predicted_time;
  // Add to integrative error
  i_error += error;

  // Update prediction
  predicted_time = predicted_time + PID_P*error + PID_I*i_error + PID_D*d_error;

  return predicted_time;
}

//define exectuion time array for oracle
#if CORE //big
    
    #if _pocketsphinx_
    int exec_time_arr[50]= {493332, 1053748, 1540036, 1307779, 1252910, 950312, 666973, 379084, 1143246, 1624743, 709901, 644991, 461722, 590817, 397468, 1232922, 427315, 1131099, 490318, 209582, 1563469, 1011618, 399950, 311084, 586049, 1056026, 1023686, 1436629, 1427039, 767212, 1703760, 1060012, 1339594, 727668, 968390, 1525803, 398910, 1151718, 1536432, 735218, 1706032, 1715809, 1148403, 1287916, 728038, 1632605, 476088, 1310558, 1477090, 1754579};
    //int exec_time_arr[50]= {2628965, 1263744, 1877792, 921323, 3359589, 2981796, 2301457, 2629482, 2705059, 1188716, 1893813, 2613509, 2193693, 805751, 2461022, 1152194, 2355681, 2429624, 1446942, 855293, 1246213, 1701739, 2295360, 3739205, 1643585, 921270, 2304967, 1888106, 1465200, 3283557, 1884156, 2326579, 1310348, 630040, 2801835, 985131, 1720021, 2955207, 3306166, 3122800, 2786696, 618042, 1258214, 1952552, 1132699, 614647, 576320, 1095123, 857723, 3038111};
    #endif
    #if _stringsearch_
    int exec_time_arr[1332]= {2488, 3087, 2493, 2347, 2919, 3026, 2483, 2583, 2781, 2556, 2672, 2816, 2737, 2759, 2281, 2400, 2686, 2279, 2257, 2390, 2191, 2334, 2886, 2330, 2621, 2486, 2247, 2366, 3236, 3337, 2267, 3086, 2275, 2346, 2226, 2586, 2287, 2456, 2527, 2326, 3183, 3638, 2246, 2591, 2651, 2536, 2641, 2286, 2281, 2498, 2794, 2386, 2451, 2346, 2322, 3066, 2482, 2466, 2308, 3022, 2421, 2471, 2612, 2366, 2321, 2361, 2916, 2337, 2529, 2281, 2497, 2491, 2247, 2653, 2207, 2171, 2676, 2385, 2567, 2412, 2522, 2781, 2396, 3188, 3086, 2429, 3034, 2392, 3053, 2567, 3186, 2527, 3191, 2416, 3001, 3026, 2357, 3188, 2504, 2636, 2510, 2556, 3108, 2331, 2557, 2821, 2697, 2366, 2595, 2669, 2492, 2353, 2551, 2385, 2591, 2394, 2916, 3061, 2332, 2820, 2778, 3011, 2623, 2452, 2373, 2891, 3053, 2447, 2575, 2817, 2352, 2713, 2782, 2774, 2731, 2308, 2356, 2709, 2236, 2281, 2346, 2215, 2306, 2910, 2301, 2623, 2487, 2285, 2366, 3234, 3363, 2242, 3111, 2467, 2372, 2191, 2521, 2407, 2494, 2487, 2363, 3194, 3611, 2288, 2566, 2678, 2511, 2639, 2286, 2308, 2467, 2781, 2411, 2411, 2383, 2296, 3035, 2457, 2507, 2281, 3048, 2396, 2497, 2587, 2403, 2282, 2400, 2876, 2377, 2491, 2308, 2471, 2639, 2559, 2616, 2243, 2171, 2679, 2347, 2627, 2387, 2548, 2612, 2536, 3296, 3251, 2544, 3161, 2544, 3026, 2605, 3201, 2637, 3289, 2516, 3187, 3197, 2487, 3336, 2616, 2723, 2568, 2557, 3107, 2332, 2563, 2821, 2671, 2367, 2596, 2631, 2518, 2326, 2551, 2393, 2592, 2402, 2917, 3060, 2331, 2808, 2751, 2780, 2626, 2452, 2371, 2891, 3053, 2446, 2585, 2781, 2622, 2710, 2782, 2783, 2731, 2306, 2356, 2724, 2237, 2296, 2347, 2233, 2307, 2915, 2302, 2613, 2486, 2277, 2366, 3239, 3376, 2242, 3114, 2467, 2373, 2191, 2530, 2406, 2580, 2552, 2465, 3322, 3737, 2432, 2676, 2710, 2586, 2657, 2312, 2282, 2498, 2757, 2425, 2411, 2385, 2296, 3023, 2456, 2494, 2282, 3045, 2397, 2495, 2587, 2391, 2281, 2398, 2921, 2336, 2531, 2281, 2499, 2466, 2273, 2616, 2245, 2171, 2678, 2346, 2605, 2386, 2548, 2757, 2421, 3156, 3119, 2428, 2996, 2430, 3027, 2592, 3162, 2526, 3191, 2516, 3186, 3187, 2486, 3328, 2616, 2675, 2542, 2656, 2905, 2487, 2667, 2822, 2656, 2366, 2585, 2631, 2515, 2326, 2588, 2361, 2630, 2361, 2955, 3048, 2331, 2810, 2751, 2872, 2601, 2495, 2346, 2929, 3053, 2446, 2576, 2782, 2611, 2671, 2821, 2774, 2731, 2308, 2356, 2713, 2237, 2287, 2347, 2232, 2307, 2929, 2301, 2613, 2487, 2274, 2366, 3222, 3360, 2242, 3191, 2466, 2378, 2191, 2569, 2246, 2496, 2486, 2352, 3156, 3637, 2282, 2566, 2692, 2512, 2655, 2287, 2308, 2466, 2781, 2386, 2436, 2346, 2320, 3066, 2481, 2466, 2281, 3063, 2397, 2509, 2586, 2392, 2281, 2386, 2877, 2374, 2491, 2321, 2472, 2505, 2246, 2643, 2207, 2199, 2637, 2371, 2566, 2412, 2522, 2782, 2396, 3189, 3126, 2391, 3034, 2391, 3072, 2593, 3162, 2551, 3166, 2415, 3001, 3002, 2394, 3157, 2506, 2636, 2511, 2556, 3109, 2354, 2531, 2846, 2632, 2390, 2556, 2656, 2492, 2363, 2552, 2400, 2591, 2400, 2916, 3021, 2359, 2781, 2778, 2764, 2626, 2452, 2372, 2929, 3026, 2486, 2551, 2819, 2561, 2699, 2781, 2763, 2756, 2281, 2382, 2686, 2262, 2256, 2383, 2192, 2346, 2887, 2339, 2587, 2512, 2246, 2392, 3196, 3361, 2242, 3086, 2427, 2347, 2228, 2586, 2283, 2457, 2526, 2327, 3182, 3638, 2246, 2591, 2651, 2535, 2617, 2313, 2282, 2500, 2756, 2425, 2411, 2389, 2296, 3093, 2456, 2499, 2281, 3191, 2525, 2527, 2543, 2366, 2317, 2361, 2914, 2336, 2534, 2281, 2498, 2466, 2271, 2616, 2231, 2171, 2661, 2346, 2592, 2386, 2557, 2756, 2447, 3198, 3087, 2418, 2996, 2414, 2946, 2604, 3202, 2527, 3206, 2391, 3028, 3029, 2356, 3181, 2467, 2733, 2471, 2579, 3107, 2332, 2570, 2821, 2681, 2367, 2588, 2631, 2515, 2326, 2576, 2361, 2619, 2361, 2940, 3061, 2331, 2820, 2752, 2801, 2601, 2478, 2346, 2914, 3053, 2446, 2576, 2782, 2594, 2671, 2818, 2736, 2731, 2319, 2356, 2713, 2237, 2281, 2347, 2227, 2307, 2925, 2301, 2629, 2486, 2272, 2366, 3219, 3337, 2242, 3130, 2466, 2386, 2191, 2523, 2407, 2555, 2486, 2354, 3156, 3644, 2291, 2567, 2690, 2512, 2642, 2286, 2305, 2466, 2802, 2387, 2449, 2346, 2335, 3066, 2483, 2466, 2309, 3054, 2396, 2510, 2586, 2406, 2281, 2386, 2876, 2360, 2491, 2305, 2471, 2491, 2246, 2642, 2206, 2203, 2637, 2387, 2567, 2424, 2521, 2783, 2397, 3181, 3113, 2391, 3090, 2391, 3050, 2567, 3161, 2565, 3166, 2430, 3001, 3027, 2382, 3157, 2492, 2636, 2493, 2556, 2898, 2331, 2567, 2863, 2631, 2406, 2556, 2657, 2491, 2352, 2552, 2385, 2592, 2389, 2916, 3047, 2367, 2781, 2791, 2764, 2640, 2452, 2373, 2891, 3094, 2483, 2551, 2818, 2587, 2715, 2782, 2763, 2755, 2281, 2391, 2686, 2279, 2256, 2385, 2191, 2307, 2912, 2324, 2587, 2520, 2246, 2407, 3271, 3443, 2242, 3118, 2435, 2346, 2216, 2586, 2270, 2456, 2514, 2327, 3187, 3651, 2247, 2606, 2652, 2540, 2616, 2310, 2281, 2491, 2757, 2411, 2412, 2370, 2297, 3064, 2456, 2504, 2281, 3135, 2423, 2472, 2609, 2367, 2319, 2362, 2917, 2337, 2531, 2282, 2499, 2466, 2272, 2617, 2235, 2172, 2661, 2347, 2590, 2386, 2553, 2756, 2438, 3196, 3086, 2418, 2996, 2415, 3027, 2603, 3199, 2527, 3206, 2392, 3028, 3024, 2357, 3263, 2466, 2675, 2471, 2596, 3081, 2331, 2555, 2821, 2665, 2366, 2602, 2632, 2529, 2326, 2577, 2361, 2616, 2362, 2950, 3060, 2332, 2822, 2752, 3038, 2601, 2477, 2346, 2924, 3066, 2446, 2589, 2781, 2616, 2671, 2806, 2769, 2731, 2320, 2356, 2736, 2237, 2284, 2347, 2215, 2307, 2920, 2301, 2630, 2486, 2285, 2367, 3222, 3358, 2241, 3122, 2467, 2385, 2192, 2535, 2246, 2484, 2486, 2353, 3156, 3640, 2285, 2566, 2691, 2512, 2644, 2287, 2306, 2466, 2780, 2387, 2436, 2346, 2321, 2997, 2488, 2467, 2319, 3065, 2397, 2498, 2587, 2417, 2282, 2398, 2876, 2375, 2491, 2320, 2471, 2491, 2246, 2638, 2206, 2198, 2636, 2371, 2566, 2410, 2521, 2790, 2396, 3196, 3114, 2391, 3094, 2392, 3053, 2566, 3198, 2566, 3167, 2431, 3002, 3028, 2383, 3156, 2490, 2637, 2496, 2556, 2915, 2332, 2567, 2861, 2632, 2394, 2559, 2660, 2492, 2355, 2551, 2395, 2592, 2388, 2916, 3046, 2368, 2781, 2790, 3011, 2639, 2451, 2373, 2891, 3027, 2484, 2551, 2820, 2562, 2709, 2782, 2764, 2758, 2281, 2387, 2686, 2276, 2257, 2387, 2191, 2337, 2886, 2330, 2587, 2514, 2246, 2392, 3271, 3362, 2242, 3086, 2446, 2347, 2230, 2496, 2433, 2531, 2577, 2326, 3193, 3649, 2246, 2606, 2651, 2536, 2616, 2313, 2281, 2493, 2756, 2410, 2412, 2373, 2297, 3099, 2457, 2495, 2281, 3061, 2424, 2472, 2613, 2367, 2318, 2361, 2916, 2336, 2530, 2281, 2497, 2467, 2272, 2616, 2238, 2172, 2674, 2347, 2605, 2386, 2552, 2756, 2421, 3180, 3086, 2414, 2996, 2418, 3027, 2598, 3202, 2526, 3205, 2391, 3028, 3027, 2357, 3256, 2466, 2660, 2471, 2582, 3116, 2331, 2569, 2821, 2672, 2367, 2586, 2631, 2519, 2326, 2577, 2362, 2615, 2361, 2940, 3060, 2332, 2821, 2752, 2798, 2601, 2479, 2346, 2917, 2986, 2446, 2576, 2781, 2612, 2671, 2819, 2736, 2731, 2321, 2357, 2712, 2237, 2283, 2346, 2227, 2306, 2926, 2302, 2625, 2486, 2276, 2366, 3222, 3337, 2241, 3125, 2466, 2389, 2192, 2524, 2406, 2482, 2486, 2359, 3156, 3655, 2285, 2567, 2678, 2511, 2640, 2287, 2316, 2466, 2796, 2387, 2451, 2347, 2322, 3066, 2482, 2466, 2306, 3046, 2396, 2497, 2586, 2397, 2281, 2399, 2876, 2376, 2491, 2310, 2471, 2494, 2247, 2652, 2206, 2211, 2637, 2386, 2567, 2413, 2521, 2782, 2397, 3189, 3126, 2391, 3034, 2391, 3057, 2567, 3187, 2558, 3166, 2429, 3001, 3040, 2383, 3157, 2492, 2881, 2497, 2557, 2891, 2331, 2561, 2853, 2631, 2406, 2567, 2669, 2491, 2353, 2551, 2386, 2591, 2388, 2917, 3046, 2357, 2781, 2787, 2764, 2639, 2452, 2384, 2891, 2976, 2472, 2551, 2805, 2587, 2696, 2781, 2763, 2767, 2282, 2396, 2687, 2275, 2256, 2372, 2191, 2306, 2911, 2337, 2586, 2526, 2246, 2406, 3197, 3362, 2242, 3115, 2462, 2347, 2229, 2497, 2444, 2456, 2512, 2326, 3182, 3612, 2246, 2593, 2651, 2539, 2616, 2319, 2281, 2495, 2757, 2426, 2412, 2375, 2296, 3022, 2457, 2503, 2282, 3065, 2396, 2511, 2612, 2366, 2307, 2362, 2908, 2337, 2532, 2281, 2537, 2467, 2272, 2616, 2232, 2171, 2668, 2347, 2605, 2387, 2559, 2757, 2425, 3156, 3112, 2423, 2996, 2430, 3026, 2607, 3103, 2527, 3111, 2391, 3032, 3002, 2399, 3195, 2467, 2809, 2471, 2586, 3082, 2357, 2532, 2853, 2671, 2366, 2596, 2631, 2520, 2326, 2577, 2361, 2624, 2361, 2958, 3060, 2331, 2808, 2751, 2917};
    #endif

    #if _sha_preread_
    int exec_time_arr[100]= {3919, 37564, 37542, 30610, 24499, 41471, 2954, 40353, 15348, 40958, 23220, 16324, 7703, 13195, 1948, 10789, 20515, 15421, 26518, 12946, 13756, 42184, 15798, 33647, 20506, 8332, 6963, 7887, 25303, 200, 39935, 37497, 42110, 20841, 16375, 24726, 4968, 25115, 1460, 22605, 44394, 32324, 11575, 20061, 33463, 27937, 24149, 38293, 19386, 28401, 34294, 35836, 22330, 21575, 34644, 44949, 14848, 2915, 14678, 11016, 20785, 36849, 6750, 10947, 41773, 19597, 41474, 39990, 18587, 4628, 43990, 40234, 3770, 20936, 25177, 41979, 22604, 19162, 27884, 9877, 22735, 20469, 35099, 31717, 11756, 20974, 41259, 23430, 12314, 21001, 18636, 19824, 11104, 2409, 28843, 27611, 3665, 20438, 32051, 5364};
    #endif

    #if _rijndael_preread_
    int exec_time_arr[100]= {7838, 8705, 17832, 10335, 2682, 16988, 14069, 10011, 14943, 4187, 2681, 180, 16040, 7798, 8102, 14472, 3680, 3066, 8010, 15120, 8991, 3112, 14097, 2656, 11869, 7136, 8978, 726, 16250, 4511, 8194, 12323, 12628, 16821, 16996, 183, 13365, 9615, 12802, 11976, 8345, 12716, 4255, 4910, 2871, 6909, 1102, 10334, 1807, 13875, 12151, 6164, 10150, 11885, 11235, 14882, 3113, 8660, 9281, 3713, 12276, 41, 3284, 3743, 14045, 16831, 3954, 11058, 11864, 8833, 8782, 11852, 549, 16372, 25722, 3519, 9223, 15294, 17660, 10935, 6163, 2217, 7976, 14335, 14595, 15167, 6300, 9701, 3412, 15990, 11450, 15869, 15322, 6486, 4305, 16606, 624, 3521, 13997, 5629};
    #endif

    #if _xpilot_slice_
    int exec_time_arr[201]= {79, 245, 86, 60, 56, 57, 54, 58, 54, 54, 53, 70, 52, 57, 54, 58, 53, 53, 53, 52, 53, 54, 53, 53, 53, 53, 55, 55, 405, 126, 124, 123, 118, 121, 117, 116, 119, 114, 112, 112, 111, 114, 113, 111, 112, 113, 113, 117, 111, 111, 116, 114, 123, 115, 116, 119, 487, 285, 299, 277, 274, 268, 273, 271, 264, 289, 270, 282, 276, 290, 274, 268, 312, 290, 266, 270, 257, 261, 260, 293, 270, 275, 282, 278, 617, 384, 362, 343, 343, 339, 327, 321, 327, 354, 320, 317, 319, 318, 315, 325, 354, 325, 324, 326, 335, 325, 332, 349, 322, 322, 326, 318, 611, 389, 406, 377, 378, 377, 368, 368, 373, 392, 370, 365, 365, 362, 364, 369, 401, 365, 361, 364, 367, 357, 373, 402, 367, 362, 364, 369, 362, 362, 400, 373, 368, 364, 368, 363, 396, 385, 360, 367, 367, 364, 364, 375, 396, 370, 369, 373, 377, 380, 383, 257, 372, 379, 382, 372, 373, 371, 487, 345, 339, 332, 323, 358, 321, 352, 324, 317, 324, 321, 328, 328, 360, 337, 328, 326, 324, 321, 327, 345, 323, 329, 331, 327, 330, 329, 353, 336, 335};
    #endif

    #if _2048_slice_
    int exec_time_arr[165]= {354, 1427, 1390, 1027, 1502, 1259, 1527, 1471, 1038, 1419, 1028, 1421, 1415, 1079, 1398, 1191, 1508, 1032, 1386, 1447, 1274, 1460, 1086, 1499, 1042, 1443, 1504, 1168, 1475, 1117, 1476, 1078, 1426, 1425, 1272, 1425, 1082, 1502, 1031, 1584, 1570, 1128, 1559, 1078, 1493, 1045, 1340, 1566, 1166, 1463, 1130, 1410, 1034, 1516, 1439, 1297, 1464, 1079, 1528, 1046, 1387, 1478, 1280, 1422, 1084, 1479, 1067, 1338, 1525, 1123, 1424, 1161, 1508, 1042, 1534, 1407, 1377, 1413, 1044, 1389, 1043, 1357, 1344, 1088, 1417, 1109, 1441, 1074, 1091, 1561, 1048, 1461, 1041, 1487, 1330, 1587, 1527, 1049, 1561, 1033, 1451, 1109, 1406, 1434, 1210, 1427, 1118, 1336, 1032, 1303, 1463, 1231, 1535, 1086, 1575, 1031, 1556, 1548, 1492, 1455, 1034, 1487, 1096, 1358, 1408, 1119, 1535, 1096, 1528, 1035, 1606, 1443, 1270, 1452, 1130, 1507, 1035, 1293, 1517, 1150, 1387, 1099, 1447, 1041, 1387, 1393, 1108, 1416, 1034, 1380, 1284, 1392, 1451, 1045, 1472, 1032, 1519, 1244, 1100, 1553, 1123, 1459, 1043, 1370, 1525};
    #endif

    #if _curseofwar_slice_
    int exec_time_arr[2002]= {995, 196, 189, 189, 187, 197, 197, 198, 197, 196, 197, 196, 197, 198, 196, 196, 197, 197, 195, 3342, 201, 200, 200, 199, 198, 201, 199, 198, 199, 198, 199, 201, 198, 198, 198, 197, 198, 199, 199, 3327, 197, 202, 200, 200, 199, 199, 201, 200, 199, 200, 199, 200, 200, 200, 199, 199, 199, 198, 201, 3353, 202, 201, 200, 201, 199, 200, 201, 199, 201, 200, 201, 200, 201, 200, 199, 200, 199, 200, 200, 3352, 202, 201, 201, 201, 200, 200, 201, 200, 200, 200, 199, 200, 200, 200, 200, 200, 201, 200, 201, 3383, 206, 202, 202, 201, 201, 203, 202, 201, 201, 202, 201, 200, 201, 201, 200, 200, 201, 202, 200, 3399, 199, 203, 203, 202, 201, 207, 204, 202, 202, 202, 201, 203, 202, 201, 201, 201, 201, 202, 201, 3307, 206, 203, 203, 203, 202, 202, 204, 207, 206, 202, 207, 203, 203, 202, 202, 202, 202, 203, 202, 3375, 205, 205, 204, 203, 203, 203, 203, 203, 204, 202, 203, 202, 202, 204, 203, 202, 203, 203, 202, 3392, 206, 205, 204, 204, 203, 203, 204, 205, 203, 203, 203, 205, 204, 203, 203, 203, 202, 204, 205, 3370, 201, 200, 205, 205, 205, 205, 204, 204, 204, 205, 204, 204, 204, 205, 204, 205, 204, 205, 204, 3429, 209, 206, 207, 206, 205, 206, 206, 205, 206, 205, 205, 205, 205, 205, 207, 205, 206, 204, 206, 3394, 208, 208, 206, 207, 206, 206, 207, 206, 206, 206, 206, 206, 206, 206, 208, 206, 208, 205, 206, 3463, 204, 208, 208, 208, 207, 207, 208, 207, 208, 206, 207, 206, 207, 207, 207, 207, 207, 207, 207, 3404, 207, 209, 209, 209, 208, 208, 210, 211, 208, 211, 208, 207, 207, 208, 208, 209, 208, 208, 209, 3468, 205, 210, 209, 210, 209, 209, 211, 208, 207, 208, 208, 208, 208, 208, 208, 208, 208, 209, 208, 3466, 206, 210, 210, 210, 209, 208, 210, 208, 207, 209, 208, 210, 208, 209, 208, 209, 209, 210, 209, 3478, 211, 211, 210, 210, 208, 210, 210, 209, 209, 209, 209, 210, 210, 209, 210, 208, 209, 209, 209, 3480, 211, 210, 210, 209, 209, 208, 211, 210, 209, 209, 209, 209, 209, 210, 209, 208, 210, 209, 209, 3537, 207, 212, 211, 211, 210, 209, 212, 210, 210, 210, 210, 209, 209, 210, 210, 210, 210, 210, 210, 3559, 213, 212, 215, 211, 211, 210, 212, 211, 211, 210, 211, 211, 211, 211, 211, 211, 210, 210, 211, 3506, 213, 216, 213, 213, 211, 212, 212, 211, 211, 212, 213, 213, 212, 212, 211, 212, 212, 213, 211, 3494, 213, 213, 213, 213, 212, 212, 212, 212, 211, 213, 211, 212, 212, 212, 212, 213, 212, 212, 212, 3507, 213, 214, 213, 213, 212, 212, 213, 212, 212, 215, 212, 211, 212, 212, 212, 212, 212, 211, 213, 3560, 214, 213, 215, 214, 213, 212, 214, 212, 213, 213, 215, 212, 213, 212, 213, 212, 212, 212, 212, 3555, 215, 214, 215, 215, 213, 213, 215, 213, 213, 213, 213, 213, 213, 213, 213, 212, 212, 215, 212, 3541, 216, 215, 215, 215, 215, 213, 214, 242, 216, 215, 214, 214, 214, 214, 213, 214, 213, 214, 214, 3555, 213, 215, 215, 216, 215, 215, 222, 214, 216, 214, 215, 214, 214, 214, 214, 215, 233, 217, 214, 3571, 213, 216, 216, 217, 215, 214, 216, 214, 216, 215, 214, 214, 215, 214, 216, 214, 215, 215, 214, 3599, 217, 216, 215, 216, 217, 215, 216, 215, 215, 215, 214, 214, 215, 214, 215, 214, 214, 215, 215, 3606, 219, 218, 217, 218, 217, 217, 217, 216, 216, 217, 217, 217, 216, 216, 217, 217, 217, 217, 217, 3627, 220, 218, 218, 218, 218, 217, 218, 216, 217, 217, 218, 217, 218, 217, 217, 217, 217, 217, 223, 3632, 221, 219, 224, 218, 217, 217, 218, 217, 220, 219, 218, 218, 217, 218, 218, 217, 217, 217, 217, 3615, 219, 221, 220, 222, 219, 219, 220, 218, 219, 218, 218, 219, 218, 218, 218, 219, 218, 219, 219, 3551, 221, 219, 221, 220, 219, 219, 220, 219, 218, 219, 219, 219, 219, 219, 218, 220, 219, 219, 219, 3601, 217, 221, 221, 221, 219, 220, 220, 219, 220, 219, 219, 220, 220, 220, 219, 220, 219, 219, 219, 3629, 220, 222, 220, 223, 219, 220, 221, 220, 219, 221, 220, 219, 219, 223, 219, 223, 219, 221, 220, 3611, 218, 225, 221, 221, 220, 220, 220, 220, 220, 220, 223, 220, 221, 220, 220, 219, 220, 220, 223, 3541, 218, 222, 222, 223, 220, 221, 222, 221, 220, 221, 220, 221, 220, 221, 220, 221, 220, 221, 220, 3636, 223, 223, 221, 222, 220, 221, 222, 220, 221, 221, 221, 220, 220, 221, 220, 221, 221, 221, 220, 3715, 219, 222, 223, 227, 221, 222, 222, 220, 221, 221, 222, 221, 221, 221, 221, 221, 221, 221, 222, 3708, 220, 223, 223, 223, 222, 222, 223, 222, 221, 224, 222, 223, 222, 223, 221, 223, 224, 223, 222, 3658, 225, 223, 223, 222, 225, 221, 222, 222, 222, 222, 222, 222, 222, 222, 222, 221, 222, 222, 222, 3686, 223, 225, 225, 224, 223, 224, 223, 224, 224, 223, 223, 222, 223, 223, 223, 222, 223, 222, 222, 3682, 222, 224, 224, 224, 224, 224, 225, 223, 223, 224, 223, 224, 224, 223, 226, 223, 224, 223, 223, 3704, 222, 225, 224, 224, 224, 224, 226, 224, 224, 224, 223, 224, 224, 224, 224, 223, 224, 224, 223, 3696, 221, 226, 224, 226, 224, 226, 226, 224, 225, 225, 224, 224, 224, 224, 224, 224, 223, 224, 224, 3759, 222, 227, 225, 226, 225, 225, 227, 225, 225, 226, 226, 225, 224, 225, 224, 224, 225, 225, 225, 3708, 227, 228, 226, 226, 226, 225, 226, 225, 225, 225, 225, 226, 224, 225, 225, 225, 225, 226, 224, 3701, 224, 228, 227, 226, 225, 226, 226, 226, 226, 225, 226, 226, 226, 226, 226, 225, 225, 227, 225, 3765, 225, 228, 227, 228, 230, 228, 230, 227, 226, 227, 227, 228, 227, 227, 227, 227, 227, 227, 226, 3773, 230, 231, 229, 228, 254, 235, 230, 228, 228, 228, 228, 228, 228, 229, 229, 227, 228, 228, 228, 3749, 226, 230, 228, 228, 228, 227, 229, 228, 228, 229, 227, 228, 227, 228, 228, 228, 228, 228, 228, 3734, 226, 230, 228, 244, 235, 230, 230, 229, 230, 230, 228, 229, 230, 229, 229, 228, 230, 229, 229, 3683, 227, 231, 230, 231, 229, 229, 229, 228, 228, 229, 230, 229, 229, 229, 228, 229, 229, 229, 229, 3794, 229, 230, 229, 230, 228, 230, 229, 228, 229, 229, 230, 228, 229, 229, 229, 228, 229, 229, 229, 3744, 227, 231, 230, 229, 229, 231, 230, 231, 230, 229, 229, 229, 229, 229, 230, 229, 230, 230, 229, 3785, 233, 232, 233, 232, 230, 230, 233, 237, 230, 231, 229, 229, 230, 230, 230, 229, 232, 231, 230, 3786, 229, 233, 231, 232, 231, 231, 234, 230, 230, 231, 230, 231, 232, 230, 231, 230, 231, 230, 231, 3835, 229, 232, 232, 231, 231, 233, 232, 231, 232, 231, 232, 230, 231, 231, 231, 231, 232, 231, 231, 3762, 230, 234, 232, 234, 233, 233, 234, 233, 232, 232, 232, 232, 232, 232, 232, 232, 234, 232, 231, 3827, 231, 235, 233, 234, 233, 233, 233, 232, 233, 234, 232, 233, 232, 232, 232, 233, 233, 233, 233, 3833, 235, 235, 232, 234, 233, 232, 235, 232, 234, 233, 232, 233, 233, 233, 233, 232, 232, 233, 233, 3813, 235, 235, 233, 234, 234, 232, 235, 233, 234, 233, 232, 233, 234, 234, 233, 234, 233, 234, 233, 3807, 237, 236, 234, 235, 235, 234, 234, 234, 234, 234, 234, 234, 234, 236, 234, 234, 234, 234, 233, 3830, 237, 236, 235, 235, 235, 235, 236, 234, 235, 236, 236, 234, 237, 235, 235, 234, 235, 235, 238, 3812, 238, 235, 235, 235, 234, 236, 234, 235, 235, 234, 235, 234, 234, 234, 234, 234, 234, 235, 234, 3820, 238, 238, 235, 237, 236, 235, 236, 235, 235, 236, 235, 235, 237, 236, 235, 235, 235, 235, 234, 3802, 232, 238, 235, 235, 236, 235, 237, 236, 235, 235, 236, 234, 235, 235, 234, 235, 235, 235, 235, 3832, 239, 238, 236, 237, 235, 237, 236, 237, 236, 237, 236, 239, 236, 236, 236, 236, 236, 237, 235, 3858, 239, 238, 236, 237, 236, 236, 236, 236, 237, 237, 235, 235, 235, 236, 236, 236, 236, 236, 236, 3876, 240, 238, 236, 238, 237, 236, 237, 236, 237, 236, 237, 236, 236, 236, 236, 236, 236, 236, 237, 3922, 238, 237, 236, 237, 237, 237, 237, 236, 240, 236, 237, 235, 236, 236, 236, 236, 236, 235, 236, 3851, 240, 240, 237, 238, 237, 237, 238, 237, 238, 237, 237, 237, 237, 236, 236, 237, 238, 236, 237, 3889, 241, 238, 237, 239, 238, 237, 238, 237, 238, 237, 236, 237, 237, 237, 236, 237, 238, 237, 237, 3944, 235, 240, 239, 239, 238, 242, 242, 239, 238, 238, 237, 238, 238, 238, 238, 238, 239, 237, 238, 3898, 241, 240, 239, 239, 238, 239, 240, 241, 238, 239, 238, 239, 238, 240, 239, 239, 238, 238, 238, 3907, 237, 241, 239, 241, 239, 239, 239, 238, 239, 239, 238, 238, 239, 239, 239, 238, 240, 239, 239, 3958, 239, 240, 238, 240, 240, 238, 239, 239, 238, 239, 238, 238, 238, 239, 239, 239, 239, 239, 238, 3876, 244, 241, 239, 240, 239, 240, 240, 239, 240, 239, 239, 239, 239, 240, 240, 239, 239, 239, 239, 3932, 242, 242, 240, 244, 241, 239, 241, 241, 241, 240, 239, 243, 240, 241, 239, 240, 240, 240, 240, 3870, 243, 242, 242, 242, 241, 241, 242, 240, 241, 241, 240, 241, 240, 241, 240, 241, 240, 241, 241, 3867, 243, 242, 241, 244, 241, 241, 242, 242, 241, 241, 240, 241, 240, 241, 241, 241, 240, 240, 240, 3900, 243, 243, 241, 243, 240, 241, 243, 241, 241, 241, 241, 240, 240, 241, 240, 241, 241, 241, 241, 3863, 239, 243, 240, 241, 240, 241, 241, 241, 241, 241, 240, 240, 241, 242, 240, 244, 241, 241, 240, 3956, 238, 243, 242, 242, 241, 241, 242, 241, 240, 240, 241, 240, 241, 240, 241, 241, 241, 241, 240, 3875, 238, 243, 241, 241, 242, 240, 242, 241, 241, 241, 240, 240, 241, 241, 240, 240, 241, 240, 240, 3954, 240, 244, 242, 244, 242, 241, 244, 241, 242, 242, 242, 242, 241, 245, 242, 244, 242, 242, 242, 3962, 239, 244, 242, 243, 242, 243, 243, 241, 241, 242, 242, 242, 245, 242, 241, 243, 242, 242, 242, 3979, 246, 244, 242, 242, 242, 242, 244, 242, 243, 243, 244, 243, 243, 242, 242, 243, 243, 242, 242, 3983, 246, 245, 244, 244, 242, 244, 244, 243, 242, 248, 242, 244, 243, 243, 243, 243, 244, 243, 243, 3942, 247, 245, 244, 246, 243, 244, 244, 243, 243, 243, 243, 243, 244, 243, 247, 243, 243, 243, 243, 3975, 241, 244, 243, 246, 243, 243, 244, 244, 244, 244, 243, 243, 244, 243, 246, 243, 243, 243, 242, 4011, 247, 245, 243, 244, 243, 243, 245, 244, 244, 243, 244, 243, 244, 244, 242, 244, 245, 243, 243, 3900, 248, 245, 244, 245, 243, 248, 245, 245, 243, 244, 243, 243, 243, 243, 244, 243, 244, 244, 244, 3943, 240, 245, 244, 245, 243, 243, 244, 243, 244, 243, 243, 243, 244, 244, 244, 244, 244, 244, 246, 3957, 249, 248, 244, 246, 244, 244, 245, 245, 245, 244, 244, 245, 245, 244, 245, 246, 245, 246, 244, 3951, 242, 246, 244, 245, 244, 245, 245, 246, 245, 245, 244, 245, 245, 244, 244, 245, 245, 246, 245, 3951, 249, 247, 245, 247, 245, 245, 246, 246, 244, 245, 244, 244, 248, 244, 245, 245, 245, 244, 244, 3985, 243, 246, 245, 245, 245, 245, 246, 245, 245, 245, 244, 246, 244, 245, 245, 248, 245, 246, 245, 3941, 244, 247};
    #endif

    #if _uzbl_
    int exec_time_arr[242]= {546, 456, 533, 454, 456, 520, 464, 530, 464, 469, 525, 471, 38, 599, 439, 394, 524, 432, 463, 708, 463, 448, 427, 485, 18, 442, 442, 418, 490, 557, 501, 20, 461, 468, 439, 425, 68, 336, 704, 181, 226, 691, 452, 535, 496, 489, 466, 452, 427, 462, 455, 616, 641, 457, 478, 432, 26056, 785, 470, 643, 21, 25319, 28, 1048, 501, 908, 775, 531, 660, 485, 19, 478, 498, 454, 433, 69, 361, 894, 169, 195, 571, 731, 521, 512, 500, 477, 488, 438, 559, 466, 25630, 854, 680, 21, 26199, 36, 1248, 498, 825, 473, 503, 550, 554, 20, 485, 439, 425, 171, 318, 850, 203, 205, 483, 734, 472, 508, 545, 440, 683, 463, 26924, 888, 554, 636, 459, 881, 508, 486, 621, 478, 18, 477, 544, 457, 433, 158, 321, 927, 177, 209, 810, 662, 463, 26359, 929, 525, 635, 462, 1395, 516, 555, 597, 479, 21, 550, 488, 444, 426, 160, 377, 872, 177, 190, 562, 499, 490, 26561, 668, 443, 591, 437, 743, 466, 499, 516, 488, 18, 468, 437, 435, 160, 356, 916, 180, 219, 469, 724, 527, 449, 27237, 982, 309, 620, 469, 489, 568, 203, 544, 440, 461, 562, 207, 531, 530, 436, 572, 197, 570, 455, 448, 542, 197, 526, 437, 444, 551, 199, 515, 446, 435, 549, 37, 538, 435, 37, 524, 422, 37, 505, 487, 35, 585, 429, 35, 502, 435, 42, 499, 424, 547, 481, 66};
    #endif   
    #if _ldecode_
        int exec_time_arr[3000]= {40297, 12563, 11943, 12282, 12706, 12855, 12278, 12353, 12640, 12342, 12802, 12651, 13153, 12687, 12991, 14608, 14370, 14694, 14516, 15205, 14556, 14266, 13943, 13416, 13198, 13472, 13601, 14257, 14615, 14698, 14583, 14505, 13952, 13815, 14300, 14670, 15060, 15202, 15167, 14957, 14694, 13689, 13328, 13005, 13593, 13639, 13469, 13944, 13471, 12822, 12782, 12339, 12606, 13635, 13264, 13155, 12949, 12468, 13357, 13258, 13769, 14621, 13444, 13805, 13773, 13438, 13949, 13884, 15200, 15314, 15370, 15395, 14980, 14270, 13286, 13287, 13808, 14035, 14696, 14861, 14912, 14259, 14220, 13788, 14441, 13522, 14326, 13461, 13657, 14294, 13514, 14053, 13631, 14041, 13925, 14252, 14243, 13504, 12708, 12756, 12794, 13152, 12889, 13520, 14108, 14778, 14289, 13578, 14702, 14222, 13817, 13565, 13703, 14214, 12629, 13428, 12728, 12754, 12639, 12746, 12178, 12776, 13168, 12918, 12743, 12805, 13190, 13350, 12773, 13093, 13840, 12970, 13073, 13089, 14516, 14652, 15905, 15188, 14985, 14318, 14185, 13666, 13084, 13082, 13489, 13000, 12950, 12953, 12797, 12922, 13768, 13837, 14082, 13716, 14216, 14710, 15362, 15145, 14749, 13800, 14422, 13734, 13249, 13378, 13455, 13208, 13202, 12552, 13198, 12934, 12400, 12635, 12301, 12571, 12136, 12360, 13106, 13501, 12943, 13243, 13166, 12891, 12578, 13186, 12682, 13179, 12995, 13160, 12900, 12605, 12825, 13173, 12508, 12209, 12739, 12224, 12654, 12908, 12438, 13493, 12793, 13161, 13066, 12667, 13107, 13429, 13340, 14301, 14852, 14680, 14573, 14786, 15251, 15469, 15493, 14976, 14530, 14649, 14202, 14316, 15047, 14655, 14091, 14474, 14018, 13964, 13823, 13190, 13565, 12946, 12996, 13102, 13037, 13080, 13905, 13991, 14229, 14249, 13990, 13793, 13171, 13711, 14202, 13899, 13410, 14247, 13477, 14703, 13475, 14451, 14277, 14204, 14212, 13773, 13522, 13123, 12257, 13318, 13321, 14102, 14415, 14426, 14446, 14273, 12831, 12540, 12507, 12883, 12659, 12807, 13201, 12367, 13453, 13409, 13802, 13926, 14155, 14427, 14998, 15302, 14874, 14008, 13501, 13965, 14253, 13657, 14280, 13386, 13313, 13912, 13824, 14232, 13647, 13483, 13829, 13735, 14427, 12862, 13437, 12602, 32677, 20295, 17662, 18016, 20305, 15107, 20548, 18218, 22219, 18985, 18976, 21494, 17392, 17148, 16963, 21016, 15648, 16867, 19051, 22185, 18599, 18168, 19833, 17281, 17947, 15210, 17413, 18148, 18203, 19065, 18734, 17787, 17351, 15828, 15405, 17464, 18505, 14976, 15352, 14291, 17339, 13274, 18439, 15737, 14523, 13898, 16861, 14549, 15395, 16574, 13507, 20203, 17963, 14399, 19798, 14615, 17988, 18971, 18115, 15939, 16644, 18093, 16779, 16958, 16341, 16106, 17727, 14944, 15390, 15801, 16891, 18278, 16461, 17380, 18719, 17077, 17110, 17420, 20741, 17052, 19662, 19186, 20690, 19529, 17876, 18027, 18602, 16788, 14935, 18789, 17180, 16299, 18482, 16596, 17185, 15760, 13922, 15268, 17106, 16804, 15806, 15304, 14833, 15935, 15020, 18928, 13356, 14818, 16203, 15896, 14232, 15576, 17487, 16102, 14990, 16788, 19343, 16980, 16306, 18618, 15294, 16208, 16991, 17217, 15027, 16884, 15485, 15736, 15917, 18558, 13951, 16099, 16915, 14030, 15039, 16790, 16833, 14897, 14924, 15333, 12873, 15569, 14094, 12864, 14029, 13622, 13670, 14089, 18827, 14500, 18861, 14111, 14189, 13091, 13679, 14068, 15165, 13579, 16852, 20535, 15720, 13291, 20891, 15050, 16763, 17349, 13949, 15780, 16105, 16155, 16165, 18018, 18411, 19601, 18697, 19112, 19242, 22427, 19086, 22548, 22204, 23715, 20571, 24175, 23734, 22493, 25580, 21353, 23978, 20818, 23876, 22466, 25067, 19999, 22093, 20058, 21238, 24680, 21767, 20738, 21711, 21487, 21755, 23000, 22866, 20044, 21924, 21341, 21629, 21485, 21343, 20262, 19666, 18924, 18236, 20608, 21122, 18819, 17191, 19201, 18582, 21953, 18914, 19289, 21453, 19040, 19450, 21455, 19710, 21442, 20625, 18956, 21195, 19028, 19105, 19765, 18100, 20463, 19282, 20480, 22990, 19432, 20853, 19013, 20701, 21132, 21659, 21913, 16457, 15905, 16702, 17185, 16789, 18130, 20458, 15949, 17820, 17892, 16531, 18890, 18516, 17013, 16069, 15358, 17806, 15887, 16738, 15696, 16362, 15284, 15585, 15546, 17619, 17341, 16947, 18221, 18339, 19818, 20906, 16815, 20725, 21248, 21278, 23695, 21267, 21867, 25172, 21650, 23450, 22842, 22205, 24286, 20963, 20931, 20381, 22719, 20037, 19026, 22498, 18156, 30900, 14515, 16127, 14678, 14161, 15232, 14266, 14973, 14880, 14400, 13720, 14458, 12505, 13145, 14275, 13863, 14934, 16041, 14083, 14152, 14108, 13090, 12728, 14939, 13351, 13791, 13115, 13238, 14119, 14707, 13408, 13937, 14747, 13353, 13749, 13659, 12904, 13652, 14851, 13221, 14411, 13677, 12791, 13264, 14417, 12667, 13232, 14777, 14177, 13726, 15655, 13245, 14522, 16404, 14030, 14408, 16028, 13789, 13857, 14948, 13754, 13668, 14461, 12730, 12719, 12932, 12927, 13972, 15410, 13925, 15068, 13893, 13226, 13619, 13749, 14084, 13687, 15180, 14255, 14297, 14215, 13348, 13049, 14428, 13470, 14163, 15288, 14286, 14466, 16406, 15677, 15636, 15816, 14462, 15458, 17241, 14926, 16162, 16849, 14291, 14632, 15378, 15146, 15210, 16202, 14947, 14759, 15974, 14789, 15171, 16078, 14969, 13810, 14727, 13530, 13973, 15296, 12928, 12658, 14214, 13121, 13392, 14046, 13244, 13352, 14070, 13589, 13600, 14980, 14077, 14521, 15263, 13462, 13329, 13962, 12965, 13003, 13942, 14667, 14149, 16092, 13930, 14567, 15809, 15064, 14617, 15020, 12552, 12913, 13668, 12896, 12804, 14622, 13518, 14633, 14653, 14196, 14665, 15452, 15187, 15037, 16078, 15216, 15587, 16111, 15365, 14463, 15390, 13642, 14631, 15558, 13429, 13385, 15011, 13341, 13454, 13894, 12939, 12825, 13582, 13290, 13626, 14077, 13160, 13165, 13354, 13045, 13283, 15320, 15244, 15289, 16116, 15306, 15453, 15556, 15151, 15965, 17819, 16365, 16925, 17778, 15023, 14480, 16027, 13151, 13565, 14317, 13372, 13701, 14385, 13352, 13032, 14776, 13504, 13575, 15461, 15009, 15469, 17543, 15780, 16644, 17432, 14670, 15542, 15926, 14241, 14627, 15974, 13558, 14224, 15089, 13133, 13598, 14085, 13193, 13779, 14120, 12749, 13309, 14025, 12811, 13573, 13795, 13341, 13516, 13657, 13100, 12778, 13932, 12946, 13404, 14478, 13687, 14144, 13692, 12900, 13002, 13783, 13365, 13740, 14428, 13859, 14193, 14827, 13815, 14387, 14211, 13793, 14758, 15636, 14574, 13820, 15212, 15126, 14747, 16303, 15184, 15154, 15782, 15017, 15445, 15860, 14534, 14976, 14981, 13752, 14616, 15227, 14700, 15592, 16809, 15772, 16462, 16444, 15046, 14979, 14549, 13876, 13895, 15626, 37068, 16368, 16073, 16235, 16970, 15610, 16434, 18558, 16347, 17314, 16403, 17651, 17027, 16807, 16967, 16606, 15660, 18484, 17415, 15856, 16306, 16147, 16014, 16448, 16691, 16261, 16199, 17603, 15618, 18651, 17783, 17517, 16241, 17462, 16839, 16988, 16988, 17509, 19356, 16633, 17587, 17250, 18056, 17200, 17438, 16837, 20285, 19786, 17929, 17927, 17767, 18769, 18577, 18812, 17747, 17410, 18070, 19596, 18917, 19507, 18965, 21515, 25248, 26140, 26683, 25401, 24725, 18669, 21047, 23260, 20268, 24207, 19387, 23050, 33356, 19207, 30045, 24820, 25545, 31499, 16039, 21934, 18695, 19859, 20052, 21548, 22925, 20141, 18241, 20729, 20089, 28652, 21603, 23142, 20477, 21281, 23174, 21992, 19278, 18924, 18126, 19775, 23264, 17959, 17959, 20156, 22255, 25415, 23173, 18390, 18713, 18583, 19428, 20061, 19610, 18810, 20274, 21109, 21313, 22890, 17457, 22454, 17456, 20624, 21826, 18381, 16731, 17640, 17569, 18478, 19584, 17840, 17598, 21301, 21591, 20995, 20930, 20164, 17755, 19549, 17624, 17338, 16509, 15264, 16790, 16130, 18807, 15883, 16867, 17963, 15849, 16404, 18456, 17792, 17772, 17323, 19653, 15668, 16044, 17007, 18441, 18179, 16322, 16376, 16951, 19218, 18155, 19684, 18100, 19726, 18693, 18596, 19605, 18435, 16250, 17996, 18517, 15927, 17189, 16920, 17732, 20018, 17981, 16837, 16363, 16499, 16116, 16569, 18434, 17544, 16372, 17071, 19685, 19301, 18216, 18304, 17483, 17393, 16286, 18393, 17169, 17682, 17367, 16146, 16702, 16419, 17494, 15952, 17121, 17589, 16870, 16841, 17142, 17327, 16532, 17364, 19724, 18445, 17633, 17328, 16869, 16432, 18073, 16791, 16657, 16558, 16596, 17837, 15693, 17025, 16632, 17582, 17171, 17092, 17037, 16633, 16362, 16704, 16437, 18033, 16860, 17748, 17217, 16562, 16877, 16234, 16851, 16483, 17005, 16361, 15780, 16604, 16190, 16472, 17628, 16936, 17151, 15692, 16131, 16168, 16901, 17053, 17056, 17481, 17085, 17667, 17317, 17420, 18008, 17084, 17881, 17584, 17611, 17977, 17275, 17912, 16034, 15946, 16469, 15645, 15612, 16364, 17224, 16338, 16699, 16032, 16348, 17142, 16686, 16043, 16243, 16953, 15451, 16748, 17935, 17460, 16513, 17278, 17653, 17749, 38196, 12788, 13234, 12552, 13424, 12767, 13472, 12752, 14118, 12751, 13787, 12680, 13741, 13416, 13755, 13647, 13293, 13249, 13976, 13245, 13389, 13063, 13637, 13622, 13410, 13312, 13400, 13546, 13247, 14024, 12906, 13945, 13013, 13815, 13268, 13154, 13812, 13829, 13443, 13351, 13349, 13502, 13793, 13308, 13929, 13122, 13672, 13268, 13822, 13522, 13180, 13541, 13461, 13788, 13610, 13655, 13429, 13809, 13246, 13548, 13388, 13690, 13849, 12975, 13320, 13972, 12764, 13725, 13384, 13340, 13823, 13078, 13410, 13840, 13021, 13675, 13429, 13486, 13650, 14116, 13589, 13191, 13333, 13568, 13502, 13415, 13175, 14038, 13387, 13984, 12992, 13892, 13335, 13420, 13750, 13729, 13332, 13775, 14273, 14094, 14880, 14494, 14832, 14476, 15394, 13466, 13567, 12869, 13789, 13610, 13421, 13576, 13524, 13535, 13570, 13568, 13724, 13639, 13863, 13336, 13914, 13929, 13324, 13427, 13968, 13501, 13713, 13697, 13848, 12773, 14286, 13812, 13329, 13007, 13994, 13147, 13630, 13478, 13810, 13632, 13732, 13903, 14411, 13230, 15348, 12869, 14219, 13174, 14461, 13690, 13779, 13485, 13584, 13216, 14227, 13327, 13654, 13156, 13444, 14908, 14948, 14083, 14681, 12988, 14155, 14100, 13725, 13244, 13143, 13638, 14122, 12836, 13908, 13297, 13890, 12718, 13502, 13346, 13516, 13308, 13845, 13238, 13947, 13723, 13328, 13529, 13206, 13383, 13928, 13532, 13959, 15621, 14409, 14491, 13741, 13896, 13112, 14457, 13415, 13466, 13743, 13787, 13095, 13511, 14030, 13579, 13455, 13248, 13535, 13642, 13686, 13450, 13505, 13544, 13361, 15357, 14759, 13543, 14611, 13975, 13555, 13605, 14103, 13279, 13276, 13910, 13124, 14096, 13369, 13333, 13923, 13336, 13820, 13146, 13495, 14178, 14922, 14454, 14721, 13852, 14104, 13711, 13883, 13508, 14165, 13713, 13785, 13943, 13897, 13910, 14282, 13994, 13589, 14299, 13716, 14549, 14484, 13674, 14277, 14183, 14682, 13813, 14519, 14393, 13921, 13891, 13952, 14207, 14036, 14412, 13829, 14183, 14239, 14299, 14148, 14446, 13971, 14041, 13221, 13425, 12987, 13762, 13296, 13727, 13146, 13492, 13109, 13695, 13239, 13823, 13239, 13551, 13428, 13553, 13274, 14055, 13315, 13122, 13430, 13749, 45103, 22173, 22764, 23389, 24535, 23251, 22655, 23991, 25388, 25569, 25867, 24934, 24817, 25596, 25647, 25590, 25917, 24616, 22970, 22765, 21434, 22705, 22090, 21513, 24711, 25031, 20788, 17177, 19319, 18879, 18503, 22672, 22175, 23220, 24895, 24886, 26577, 25807, 26802, 23400, 21876, 22290, 22653, 20560, 22277, 22137, 22696, 23934, 27311, 23906, 23153, 22817, 21140, 20695, 22846, 25440, 27590, 25676, 24107, 21065, 24050, 24493, 23248, 24005, 26540, 26769, 28090, 27686, 25213, 23265, 24967, 24279, 23034, 21587, 23647, 24243, 24756, 25453, 24805, 25034, 25266, 24916, 23581, 24002, 25038, 25413, 26012, 24145, 25033, 25772, 28087, 28525, 27567, 27055, 29766, 25211, 22951, 20523, 21008, 21235, 19265, 22009, 22473, 22089, 23751, 23368, 19012, 15752, 20929, 22678, 21425, 20844, 21566, 22439, 20648, 21896, 21039, 17283, 19923, 21152, 19269, 19550, 21698, 22323, 23851, 23697, 26360, 23906, 23912, 23322, 23091, 23816, 25227, 24151, 23202, 24718, 25784, 25851, 30787, 27380, 29727, 26694, 25779, 23268, 24376, 27978, 24261, 26839, 23509, 21361, 23624, 26714, 26843, 30175, 30599, 31445, 31977, 29986, 28710, 24389, 24311, 22616, 22857, 19886, 22833, 20455, 22256, 23591, 23265, 24499, 25586, 27347, 28855, 30440, 29600, 28662, 29986, 28530, 28244, 28081, 27620, 27687, 28383, 29159, 29786, 28338, 29086, 28437, 26770, 28933, 28582, 30317, 30654, 29240, 29277, 31205, 31224, 27802, 25343, 27679, 27860, 25129, 27656, 25416, 23365, 26044, 23751, 24003, 22921, 26523, 26783, 24339, 27829, 25781, 26448, 27663, 26331, 27394, 26253, 30209, 27861, 30919, 28014, 30309, 22936, 21864, 31235, 31778, 28297, 30178, 25916, 33157, 26731, 21772, 29622, 29896, 19459, 19323, 31334, 32204, 30776, 26160, 20906, 19851, 21782, 25102, 22061, 22269, 22307, 22441, 17932, 20663, 17369, 16308, 20094, 23995, 29752, 24073, 18707, 19232, 20459, 22140, 22252, 20581, 18099, 19079, 20253, 19975, 21684, 18392, 16313, 17823, 19699, 17719, 16152, 17093, 15449, 18501, 18149, 19802, 17629, 14386, 17884, 21808, 19028, 22464, 22797, 20589, 20548, 19452, 18397, 20127, 19907, 18907, 21252, 20630, 21673, 21055, 18752, 16179, 36057, 12551, 11919, 12209, 12282, 12340, 12520, 12059, 12219, 12293, 12258, 12177, 12042, 12107, 12404, 12516, 12907, 12817, 12984, 13309, 13424, 13644, 14031, 13876, 14476, 14160, 14047, 14277, 14613, 14672, 14358, 14622, 14422, 14659, 14244, 14318, 14446, 13992, 14534, 14065, 14029, 14369, 13977, 14574, 14137, 14163, 14623, 13993, 14492, 14568, 14394, 14622, 14504, 13972, 14313, 14072, 14211, 14122, 14246, 13885, 14045, 13985, 14394, 14447, 13919, 13875, 13895, 13624, 14224, 13767, 13845, 13589, 13500, 13516, 13640, 13368, 13602, 14288, 14077, 13737, 13726, 13726, 14206, 13826, 13514, 14351, 13829, 14071, 14410, 14637, 14421, 14210, 14381, 13634, 14122, 13875, 14041, 13982, 14140, 13926, 13847, 14001, 13803, 14063, 13631, 13679, 14161, 13522, 13940, 13628, 13490, 13672, 13502, 13536, 13350, 12864, 13245, 12989, 13141, 13348, 13493, 13252, 13007, 13322, 13351, 13557, 13357, 13148, 13398, 13177, 13501, 13650, 14048, 13571, 13534, 13776, 13877, 13847, 13490, 13178, 13795, 13460, 13026, 13972, 13535, 13438, 13351, 13045, 13846, 13702, 13888, 13912, 14018, 13773, 13511, 13743, 13262, 13574, 13373, 13618, 13591, 13489, 13877, 13710, 13783, 13568, 13695, 14005, 13431, 14085, 13553, 13756, 13779, 13690, 13816, 13591, 13543, 13497, 13425, 13839, 13341, 13480, 13423, 13855, 13352, 14092, 13677, 13805, 13575, 13540, 13183, 13197, 13644, 13479, 13288, 13566, 13577, 12833, 13279, 13540, 13500, 13450, 13743, 13769, 14385, 14033, 13855, 13938, 13792, 13828, 13556, 14228, 13647, 13555, 13784, 13619, 13371, 14182, 13777, 13690, 14112, 13840, 14426, 14037, 13561, 13955, 13624, 13522, 14067, 14493, 13682, 14177, 13660, 14246, 13996, 14324, 14179, 13814, 14301, 13802, 13704, 13792, 13667, 13870, 13660, 14131, 13659, 13651, 13506, 13234, 13193, 13285, 13145, 13164, 13256, 13151, 13301, 13235, 12988, 13282, 13334, 13812, 13133, 13625, 13346, 13393, 12945, 13808, 13658, 13826, 13920, 13734, 13608, 13487, 13966, 13854, 14078, 14094, 14280, 14297, 14154, 14008, 14090, 13956, 13596, 13931, 13836, 14036, 13901, 13719, 13974, 14017, 14049, 14526, 13781, 14234, 14358, 13842, 14093, 14387, 45295, 14820, 15266, 13334, 13913, 14974, 15127, 14539, 13196, 14212, 13890, 12787, 14006, 13791, 14049, 13696, 13526, 13774, 13453, 13195, 14332, 13349, 14386, 15367, 16699, 18535, 20006, 20394, 21571, 21129, 20359, 19325, 18541, 19042, 16407, 16113, 17084, 14925, 16269, 15901, 16068, 15735, 17046, 18229, 19397, 18953, 19615, 20507, 20390, 19862, 20729, 20029, 18911, 20202, 18876, 18428, 16625, 17470, 17302, 18567, 19422, 20989, 21079, 20987, 22176, 21305, 20500, 20500, 19841, 18983, 18174, 17677, 18059, 17995, 17313, 17926, 19804, 19787, 18591, 18803, 20780, 18011, 18693, 18828, 19755, 17537, 17860, 17426, 18671, 19181, 19269, 20565, 19604, 19630, 20360, 19507, 19647, 18604, 19841, 19202, 18767, 18844, 18434, 17408, 19399, 18693, 19292, 17065, 17650, 17996, 18824, 17900, 17514, 17347, 17335, 17025, 19019, 18261, 18830, 18387, 18948, 18741, 20795, 18940, 18101, 18584, 19539, 20874, 21406, 21473, 21227, 21022, 20608, 19476, 20493, 17804, 16609, 16246, 16932, 17919, 18378, 16494, 16217, 16174, 17798, 18721, 20055, 19600, 21356, 21275, 23271, 22237, 22912, 23385, 23704, 22771, 21084, 21221, 20585, 20396, 20389, 19364, 21154, 19577, 19787, 18272, 19606, 18953, 20636, 19499, 20951, 20884, 21169, 20102, 20582, 19831, 18100, 17027, 16281, 17656, 17620, 16241, 18766, 17783, 19304, 19687, 21568, 20690, 21920, 21381, 24192, 22466, 22569, 21669, 21886, 21037, 21109, 20547, 20320, 20036, 20380, 19006, 19966, 18950, 19237, 18380, 20470, 19161, 18970, 17394, 16981, 17333, 17405, 16787, 16156, 16356, 16812, 15765, 16043, 15312, 15321, 14719, 14840, 14386, 14032, 14224, 14199, 14525, 14719, 15038, 16951, 17434, 18170, 19224, 17851, 15737, 16074, 14309, 13782, 13422, 14009, 14031, 14843, 16080, 15659, 15388, 16198, 15278, 16098, 15529, 16071, 15452, 15786, 15678, 15743, 14539, 16931, 18208, 18038, 19095, 21779, 19663, 18962, 17840, 17738, 17129, 18518, 17232, 16499, 16602, 17285, 17134, 16405, 13564, 13545, 14278, 15250, 13571, 14087, 15043, 13957, 13209, 14398, 13450, 14459, 14504, 14223, 13850, 13526, 13756, 14232, 14511, 14697, 14234, 15385, 15032, 15126, 16225, 17005, 16438, 47877, 17379, 16125, 12407, 16439, 16783, 16287, 16506, 12591, 17413, 17754, 17116, 16915, 13548, 17242, 16956, 17001, 15825, 13597, 17044, 17412, 16801, 17043, 12968, 17029, 16914, 16623, 17714, 12949, 17153, 17017, 16757, 16672, 12605, 16515, 16526, 16260, 15932, 12369, 16617, 16222, 16518, 16520, 12917, 16394, 16255, 16219, 17508, 12185, 16193, 16446, 16331, 16645, 12504, 16305, 16477, 17811, 16889, 12760, 17224, 16703, 17880, 17751, 13761, 17539, 17226, 16951, 17428, 13206, 17473, 17636, 18240, 17590, 13610, 18455, 18357, 18628, 19683, 15007, 19639, 19031, 20073, 20586, 15908, 20038, 20667, 20700, 20223, 16187, 21334, 25044, 21033, 20405, 16106, 19757, 19270, 19605, 19353, 14414, 18478, 17878, 17569, 18015, 14252, 18284, 18543, 18660, 18975, 14050, 18073, 17614, 16910, 17253, 13112, 17283, 16483, 16596, 16573, 12452, 16144, 16387, 16171, 16621, 12375, 16146, 16375, 16938, 17072, 13478, 18098, 18757, 18028, 19154, 14249, 18378, 18761, 18055, 19286, 13177, 18642, 18439, 18476, 17784, 13687, 18527, 17615, 17534, 17341, 12847, 17295, 23895, 17471, 17040, 13440, 16979, 17106, 17125, 16991, 12916, 16244, 17424, 17435, 16059, 13139, 16585, 16896, 17694, 17887, 13484, 18829, 18446, 18440, 18759, 15006, 17541, 18709, 19244, 19041, 15228, 19625, 18695, 19005, 20477, 15904, 19009, 18862, 19929, 18484, 14802, 18091, 17726, 17699, 17428, 13053, 16572, 17547, 16567, 16830, 13338, 16346, 16454, 16219, 16896, 13058, 16348, 17018, 17536, 17551, 12882, 16659, 16101, 16913, 16648, 12844, 17625, 16806, 17005, 17606, 13683, 17623, 17658, 17833, 17817, 13253, 18061, 18230, 18857, 19266, 14865, 18406, 18301, 18839, 18854, 14422, 17739, 18811, 17964, 17913, 14175, 18733, 22500, 18202, 17643, 12904, 17291, 17173, 17140, 17323, 13205, 16807, 16871, 16637, 17031, 13110, 17117, 17025, 18159, 18287, 13302, 18666, 18535, 18003, 17978, 13556, 17020, 17752, 17113, 17470, 13357, 17302, 17820, 17732, 17028, 13412, 16858, 17834, 17200, 17798, 12782, 17213, 17460, 17091, 17497, 12645, 17552, 18069, 16817, 17224, 12803, 17765, 17194, 17556, 17122, 12983, 17630, 16480, 16930, 17365, 13546, 17575, 46434, 12299, 12350, 12269, 12483, 13141, 14081, 13492, 13854, 15697, 15543, 16686, 16674, 17428, 17411, 17957, 17844, 18250, 16654, 16154, 15995, 14200, 14069, 13870, 14523, 15687, 17847, 18609, 19897, 16294, 16627, 16219, 16236, 16209, 16250, 16849, 16028, 16889, 15584, 15443, 16950, 16281, 17715, 17965, 17922, 17377, 17021, 15674, 16146, 17668, 17930, 18950, 17557, 17186, 17369, 18077, 17042, 15236, 15290, 14984, 16410, 16402, 17308, 18121, 18503, 19240, 18509, 17792, 17478, 17495, 16848, 16522, 16540, 16912, 16956, 16756, 18004, 17988, 18140, 17860, 17348, 17123, 16147, 16146, 16140, 16933, 17063, 16756, 15936, 15230, 14515, 13745, 14403, 13833, 13454, 14096, 15057, 15710, 16017, 15420, 15459, 15819, 16492, 15485, 13121, 14594, 15286, 16367, 16324, 16719, 17543, 17942, 17284, 17243, 18083, 19370, 19235, 17731, 18744, 17167, 18211, 17196, 16936, 16720, 17673, 17890, 16508, 16849, 15374, 17313, 16307, 15559, 15797, 15102, 15500, 14865, 15004, 15083, 14953, 15405, 15674, 15677, 15856, 15514, 16238, 14709, 13854, 14200, 12835, 12604, 12796, 12879, 13517, 15000, 16212, 15968, 17046, 18044, 17339, 16885, 16867, 16161, 16516, 16469, 16538, 17602, 17813, 18269, 19107, 19147, 19041, 17859, 17387, 16062, 16518, 17729, 18193, 18256, 18032, 17793, 17727, 17540, 16647, 16616, 16282, 16889, 17210, 17064, 16574, 16873, 18352, 18825, 18644, 18947, 18982, 20794, 19539, 19425, 18847, 19558, 18933, 18181, 17696, 18445, 19293, 20915, 21466, 20401, 20966, 20875, 19873, 18372, 18264, 16994, 16814, 17654, 17458, 18230, 18584, 19381, 19766, 18862, 18499, 19754, 17737, 16454, 13981, 14438, 14372, 14991, 14874, 13765, 14310, 14065, 16768, 16864, 18094, 18612, 19767, 19498, 19789, 18855, 18468, 18055, 17128, 16194, 17495, 18008, 17953, 17552, 16053, 16449, 16516, 17127, 16710, 17376, 17357, 17724, 16839, 18842, 19305, 18965, 17758, 17930, 19097, 20347, 20283, 19608, 19602, 18409, 17727, 17432, 16237, 14765, 15842, 15463, 16953, 15291, 14146, 14106, 14255, 13775, 15404, 14235, 14902, 14755, 14896, 15937, 14313, 15300, 15996, 13913, 13837, 14934, 13661, 13957, 14797, 15234, 15453, 14739};
    #endif

//define exectuion time array for oracle
#else //LITTLE
    #if _pocketsphinx_
    int exec_time_arr[50]= {1011979, 788292, 1590383, 1803987, 1042574, 1186525, 2048139, 1627050, 2498808, 2408511, 987810, 1646961, 1340375, 2068592, 802988, 1945818, 2670511, 2469172, 1118919, 2733652, 3088226, 950868, 1890375, 1599193, 1018672, 2482998, 890064, 1157446, 2147644, 1645368, 923497, 2400959, 1800026, 798730, 939022, 1302031, 692731, 2698502, 2794897, 1559726, 1558239, 1601462, 1099571, 1510033, 1384929, 2235707, 1452028, 1060810, 2402230, 1048889};
    #endif

    #if _stringsearch_
    int exec_time_arr[1332]= {7666, 8091, 8143, 7675, 8087, 7765, 7760, 8343, 8110, 7503, 8034, 8053, 8191, 7421, 7359, 7369, 7581, 7079, 7262, 7272, 7178, 7368, 8251, 7230, 7814, 7638, 7115, 7484, 8065, 8445, 7204, 8077, 7070, 7874, 7160, 7708, 7346, 8009, 7875, 7429, 8178, 8821, 7172, 7523, 8382, 7955, 8039, 7387, 7468, 7187, 7843, 7539, 7592, 7202, 7292, 7977, 7472, 7285, 7654, 7959, 7393, 7486, 7484, 7594, 7362, 7745, 9110, 7571, 8128, 7365, 7586, 7203, 7230, 7726, 7162, 6863, 8090, 7491, 8219, 7620, 7426, 7937, 7743, 8174, 8041, 7257, 7939, 7314, 7992, 7552, 7959, 7614, 7983, 7706, 7879, 7821, 7331, 8130, 7206, 8115, 8054, 7892, 8606, 7314, 7841, 8555, 7883, 7552, 8055, 7944, 8014, 7409, 7319, 7674, 8094, 7699, 8939, 8368, 7443, 7999, 8167, 8185, 8032, 8058, 7572, 7977, 7733, 7751, 8340, 8070, 7783, 8018, 8054, 8224, 7430, 7662, 7488, 7547, 7092, 7307, 7271, 7296, 7358, 8304, 7254, 7788, 7654, 7100, 7517, 8034, 8384, 7186, 8094, 7101, 7572, 7058, 7463, 7228, 7914, 7867, 7425, 8161, 8818, 7091, 7542, 8507, 7766, 8012, 7328, 7471, 7180, 7839, 7533, 7564, 7221, 7335, 7900, 7465, 7309, 7399, 8133, 7395, 7578, 7474, 7727, 7386, 7719, 9151, 7587, 8023, 7362, 7522, 7200, 7227, 7766, 7124, 6858, 8206, 7296, 8049, 7643, 7422, 7831, 7747, 8191, 8012, 7248, 8010, 7285, 7755, 7481, 8120, 7597, 7991, 7280, 7805, 7849, 7295, 8155, 7217, 8194, 8070, 7901, 8600, 7350, 7800, 8583, 8080, 7573, 8608, 8126, 8028, 7498, 7317, 7690, 8088, 7704, 8938, 8340, 7365, 7985, 8179, 8147, 7939, 8047, 7706, 7944, 7735, 7754, 8340, 8063, 7458, 8017, 8059, 8238, 7422, 7337, 7456, 7487, 7077, 7285, 7276, 7214, 7369, 8230, 7226, 7789, 7649, 7150, 7471, 9499, 8740, 7240, 8301, 7236, 7629, 7081, 7349, 7432, 7981, 7850, 7493, 8141, 8913, 7082, 7465, 8208, 7698, 7994, 7321, 7398, 7202, 7835, 7599, 7557, 7307, 7265, 7982, 7444, 7316, 7372, 7975, 7414, 7499, 7476, 7615, 7374, 7729, 9202, 7607, 8028, 7397, 7479, 7217, 7234, 7794, 7216, 6908, 8065, 7343, 8050, 7726, 7446, 7923, 7791, 8134, 8030, 7252, 7934, 7311, 7724, 7505, 7966, 7607, 8057, 7305, 7843, 7800, 7313, 8134, 7232, 8120, 8077, 7904, 8596, 7362, 7905, 8545, 7839, 7547, 8081, 7964, 8001, 7405, 7318, 7691, 8081, 7700, 8940, 8446, 7372, 7975, 8128, 8196, 7930, 8030, 7633, 7959, 7716, 7768, 8328, 8165, 7505, 8235, 8096, 8303, 7628, 7370, 7647, 7545, 7326, 8891, 7965, 7178, 7508, 8247, 7245, 7820, 7620, 7109, 7494, 8078, 8334, 7224, 8137, 7114, 7699, 7044, 7373, 7206, 8011, 7824, 7445, 8134, 8793, 7117, 7482, 8231, 7757, 8038, 7363, 7375, 7220, 7817, 7590, 7548, 7252, 7273, 7989, 7439, 7339, 7398, 8022, 7407, 7460, 7526, 7589, 7438, 7729, 9129, 7624, 8012, 7414, 7472, 7252, 7291, 7773, 7132, 6916, 8070, 7277, 8138, 7593, 7458, 7834, 7788, 8170, 8035, 7357, 7904, 7412, 7714, 7533, 7987, 7576, 8043, 7250, 7852, 7823, 7346, 8281, 7191, 8183, 8041, 7879, 8643, 7321, 7825, 8595, 7796, 7586, 8076, 7948, 8135, 7380, 7355, 7644, 8094, 7714, 8940, 8383, 7373, 7955, 8175, 8144, 8064, 8090, 7585, 7977, 7922, 7906, 8426, 8111, 7457, 8515, 8071, 8304, 7600, 7304, 7423, 7595, 7422, 7352, 7325, 7198, 7418, 8228, 7300, 7830, 7631, 7205, 7510, 8071, 8339, 7240, 8083, 7092, 7666, 7033, 7383, 7217, 7978, 7813, 7540, 8172, 8834, 7119, 7494, 8213, 7689, 8008, 7343, 7359, 7226, 7812, 7560, 7636, 7264, 7942, 8223, 7547, 7625, 7479, 7949, 7585, 7519, 7535, 7644, 7365, 7856, 9086, 7575, 8067, 7365, 7537, 7192, 7256, 7733, 7156, 6901, 8098, 7257, 8191, 7637, 7423, 7849, 7746, 8179, 8038, 7246, 7942, 7284, 7757, 7474, 8089, 7604, 8114, 7294, 7811, 7840, 7289, 8172, 7197, 8342, 8088, 7853, 8628, 7451, 7814, 8576, 7826, 7544, 8095, 7945, 8042, 7407, 7319, 7679, 8067, 7800, 8981, 8347, 7415, 7966, 8126, 8190, 7930, 8072, 7624, 7944, 7748, 7734, 8467, 8117, 7440, 8097, 8044, 8184, 7474, 7305, 7411, 7480, 7173, 7263, 7381, 7191, 7404, 8198, 7279, 7810, 7637, 7139, 7480, 8080, 8338, 7249, 8069, 7180, 7629, 7062, 7332, 7233, 7939, 7826, 7464, 8149, 8822, 7139, 7546, 8212, 7771, 8018, 7345, 7375, 7252, 7813, 7580, 7554, 7260, 7267, 7949, 7440, 7324, 7443, 7963, 7415, 7470, 7525, 7590, 7414, 7700, 9136, 7668, 8012, 7414, 7490, 7321, 7205, 7819, 7126, 6893, 8108, 7312, 8082, 7600, 7455, 7806, 7783, 8152, 8094, 7284, 7900, 7322, 7732, 7504, 7959, 7630, 8030, 7237, 7868, 7806, 7404, 8148, 7340, 8156, 8051, 7902, 8617, 7337, 7832, 8560, 7773, 7591, 8060, 8040, 8053, 7387, 7342, 7652, 8095, 7678, 8966, 8363, 7387, 7994, 8147, 8241, 7941, 8101, 7599, 7973, 7712, 7776, 8366, 8057, 7479, 8012, 8073, 8300, 7416, 7346, 7396, 7793, 7092, 7292, 7276, 7225, 7362, 8240, 7250, 7779, 7736, 7102, 7518, 8052, 8383, 7204, 8075, 7142, 7589, 7071, 7801, 7337, 7975, 7985, 7443, 8184, 8873, 7107, 7514, 8200, 7722, 7979, 7297, 7437, 7184, 7851, 7623, 7580, 7225, 7341, 7931, 7430, 7330, 7371, 7929, 7394, 7518, 7468, 7620, 7458, 7737, 9130, 7625, 8044, 7374, 7486, 7225, 7213, 7761, 7126, 6869, 8112, 7706, 8103, 7635, 7466, 7940, 7878, 8229, 8030, 7254, 7951, 7307, 7734, 7603, 7953, 7609, 8011, 7261, 7854, 7872, 7324, 8135, 7229, 8130, 8110, 7931, 8712, 7346, 7911, 8585, 7833, 7583, 8068, 7953, 8009, 7420, 7325, 7664, 8193, 7694, 8930, 8372, 7410, 7995, 8211, 8150, 7929, 8037, 7592, 7962, 7730, 7866, 8360, 8067, 7506, 8031, 8052, 8228, 7411, 7318, 7393, 7640, 7117, 7416, 7284, 7215, 7374, 8287, 7244, 7818, 7671, 7084, 7511, 8046, 8368, 7195, 8171, 7082, 7624, 7025, 7405, 7215, 7941, 7876, 7423, 8182, 8847, 7089, 7517, 8267, 7759, 8001, 7346, 7446, 7188, 7837, 7551, 7580, 7209, 7319, 7934, 7481, 7366, 7411, 7914, 7396, 7520, 7469, 7628, 7372, 7728, 9136, 7602, 8032, 7392, 7574, 7182, 7256, 7740, 7191, 6878, 8122, 7285, 8096, 7681, 7430, 7851, 7771, 8250, 8034, 7246, 7935, 7318, 7822, 7490, 7981, 7588, 7988, 7275, 7816, 8033, 7309, 8173, 7200, 8136, 8078, 7861, 8634, 7370, 7787, 8587, 7839, 7547, 8172, 7937, 8115, 7404, 7368, 7666, 8056, 7749, 8936, 8330, 7434, 8008, 8203, 8190, 7975, 8050, 7651, 7943, 7749, 7723, 8332, 8077, 7457, 8085, 8122, 8205, 7459, 7294, 7416, 7481, 7129, 7270, 7314, 7177, 7415, 8209, 7241, 8100, 7640, 7106, 7497, 8083, 8396, 7204, 8083, 7090, 7629, 7051, 7392, 7201, 8056, 7822, 7438, 8302, 8803, 7140, 7489, 8229, 7734, 7966, 7348, 7370, 7245, 7896, 7590, 7561, 7240, 7283, 7962, 7464, 7304, 7407, 7898, 7444, 7515, 7506, 7666, 7414, 7691, 9138, 7641, 8411, 7367, 7521, 7192, 7246, 7728, 7160, 6861, 8203, 7283, 8090, 7599, 7475, 7836, 8097, 8204, 8004, 7285, 7948, 7304, 7828, 7494, 7985, 7577, 8017, 7251, 7910, 7837, 7300, 8175, 7188, 8148, 8077, 7936, 8639, 7342, 7785, 8592, 7805, 7706, 8419, 8008, 8083, 7365, 7329, 7776, 8094, 7821, 8964, 8338, 7415, 8019, 8279, 8189, 7905, 8039, 7619, 7933, 7852, 7720, 8349, 8113, 7452, 8091, 8063, 8197, 7586, 7323, 7424, 7474, 7209, 7289, 7306, 7191, 7414, 8244, 7244, 7813, 7631, 7124, 7776, 8972, 8499, 7291, 8118, 7119, 7680, 7038, 7400, 7240, 7960, 7867, 7435, 8175, 8828, 7233, 7594, 8176, 7747, 7997, 7320, 7423, 7193, 8118, 7602, 7616, 7223, 7311, 8057, 7572, 7296, 7386, 7941, 7412, 7525, 7504, 7615, 7367, 7736, 9205, 8085, 8747, 7811, 7616, 7293, 7214, 7805, 7159, 6951, 8093, 7307, 8121, 7595, 7465, 7908, 7781, 8135, 7993, 7292, 7942, 7333, 7742, 7511, 7985, 7592, 8042, 7242, 7918, 7821, 7348, 8177, 7180, 8203, 8041, 7906, 8642, 7335, 7817, 8652, 7864, 7570, 8071, 8333, 8149, 7404, 7354, 7660, 8288, 7796, 8947, 8363, 7492, 7980, 8256, 8205, 7925, 8070, 7597, 7997, 7764, 7720, 8371, 8103, 7458, 8167, 8015, 8229, 7460, 7400, 7433, 7585, 7143, 7286, 7314, 7194, 7409, 8210, 7376, 7781, 7657, 7096, 7515, 8226, 8444, 7236, 8056, 7128, 7717, 7112, 7461, 7362, 8204, 7850, 7470, 8380, 8806, 7141, 7524, 8219, 7741, 7992, 7363, 7459, 7228, 7812, 7602, 7545, 7243, 7272, 7939, 7464, 7295, 7362, 7903, 7425, 7551, 7524, 7579, 7407, 7703, 9138, 7639, 8012, 7430, 7463, 7269, 7222, 7993, 7216, 6883, 8385, 7273, 8134, 7604, 7462, 7918, 7837, 8205, 8001, 7283, 7907, 7400, 7775, 7483, 7987, 7579, 8036, 7258, 7855, 7896, 7302, 8178, 7198, 8243, 8071, 7851, 8622, 7335, 7933, 8659, 7802, 7601, 8109, 7932, 8052, 7381, 7429, 7662, 8118, 7725, 8911, 8367, 7427, 7968, 8473, 8198};
    #endif
    #if _sha_preread_
    int exec_time_arr[99]= {43734, 24210, 9284, 12991, 34611, 13364, 31839, 32101, 38787, 4621, 29221, 41708, 20301, 11669, 32366, 33253, 10082, 25904, 23448, 11025, 8580, 33526, 7320, 17130, 39368, 44307, 38428, 22407, 39688, 36073, 33024, 31533, 39616, 26257, 11543, 19387, 20141, 17852, 32038, 7781, 8653, 30499, 8033, 19827, 10295, 13142, 17761, 22419, 43177, 8148, 36419, 32884, 17401, 39668, 39076, 25458, 43392, 12308, 18531, 31883, 40735, 20408, 24527, 18780, 10747, 17856, 13773, 27161, 20198, 19908, 25254, 17326, 20789, 40573, 38754, 30035, 13148, 21746, 26041, 22760, 23825, 41211, 22420, 7821, 10769, 38567, 45135, 24483, 19413, 25651, 44787, 36768, 14250, 15398, 42833, 7983, 27539, 28504, 34127};
    #endif

    #if _rijndael_preread_
    int exec_time_arr[99]= {23948, 44612, 40693, 19279, 27253, 30480, 14456, 22047, 35261, 15599, 29239, 31262, 29628, 18152, 25361, 24502, 38811, 39974, 16248, 37790, 37444, 34906, 15539, 39964, 16536, 30630, 16676, 21493, 14742, 27581, 27040, 20048, 31154, 19582, 15739, 35339, 30868, 14868, 41224, 23129, 34151, 22202, 20041, 40349, 20421, 14890, 26211, 40647, 24870, 57863, 26013, 20105, 39726, 35155, 19042, 29111, 42482, 41724, 36705, 14530, 26726, 15700, 42346, 36402, 40269, 32765, 29995, 35013, 29782, 36139, 14905, 32595, 23326, 32067, 16313, 21229, 22712, 15880, 25926, 42167, 27938, 18773, 39952, 41806, 23218, 33737, 40311, 36508, 26845, 43379, 41243, 26595, 17798, 29160, 22442, 42716, 18902, 28387, 40101};
    #endif

    #if _xpilot_slice_
    int exec_time_arr[201]= {163, 492, 236, 187, 195, 172, 171, 198, 183, 171, 169, 221, 168, 170, 172, 167, 170, 177, 161, 164, 168, 170, 167, 184, 164, 171, 182, 168, 1058, 803, 805, 786, 772, 747, 785, 746, 750, 730, 722, 721, 712, 778, 741, 706, 744, 739, 726, 744, 775, 743, 725, 746, 737, 794, 738, 802, 1515, 1003, 994, 983, 954, 914, 994, 940, 931, 919, 927, 972, 952, 1264, 1038, 960, 1015, 991, 937, 926, 962, 918, 913, 939, 967, 970, 952, 1005, 1883, 1633, 1225, 1208, 1179, 1202, 1205, 1144, 1162, 1142, 1168, 1163, 1115, 1149, 1121, 1157, 1174, 1142, 1126, 1138, 1199, 1135, 1127, 1140, 1126, 1114, 1121, 1153, 1924, 1357, 1377, 1309, 1285, 1263, 1307, 1296, 1284, 1306, 2824, 1284, 1273, 1282, 1264, 1230, 1243, 1249, 1284, 1242, 1271, 1303, 1275, 1373, 1301, 1243, 1257, 1507, 1297, 1290, 1319, 1291, 1326, 1302, 1337, 1268, 1322, 1270, 1274, 1279, 1247, 1310, 1261, 1315, 1283, 1309, 1328, 1300, 1329, 1321, 1357, 1304, 1315, 1342, 1319, 1337, 1352, 1299, 1578, 1238, 1181, 1166, 1213, 1263, 1120, 1139, 1172, 1138, 1164, 1182, 1189, 1161, 1195, 1198, 1149, 1151, 1138, 1189, 1161, 1159, 1139, 1150, 1169, 1177, 1170, 1201, 1522, 1183, 1185};
    #endif

    #if _2048_slice_
//    int exec_time_arr[165]= {1970, 2026, 1789, 1941, 1778, 1927, 1852, 2004, 1881, 1809, 1828, 1442, 1931, 1418, 1922, 1801, 1872, 1791, 1920, 2030, 2001, 1920, 2005, 1975, 1795, 1842, 1478, 1857, 1417, 1940, 1810, 1948, 1782, 1958, 1823, 1936, 1862, 1982, 1980, 2286, 1993, 1883, 1871, 1429, 1857, 1403, 1879, 1754, 1974, 1871, 1906, 1878, 1907, 1860, 1932, 1870, 1869, 1925, 1460, 2207, 1433, 1979, 1406, 1986, 1897, 1899, 1890, 1937, 1848, 1923, 1829, 1969, 1923, 1537, 1839, 1423, 1858, 1412, 2008, 1649, 1881, 1867, 1913, 1954, 1882, 1824, 1927, 1893, 1883, 1885, 1481, 1929, 1419, 1971, 1759, 1930, 1860, 2045, 1849, 1910, 1808, 1920, 1922, 1927, 1896, 1450, 1842, 1425, 1901, 1485, 1922, 1848, 1879, 1911, 1927, 1862, 1893, 1834, 1884, 1971, 1476, 1888, 1420, 1954, 1763, 1880, 1923, 1930, 1834, 1958, 1841, 1924, 1885, 1914, 1913, 1481, 1800, 1423, 1899, 1416, 1976, 1923, 1933, 1846, 1944, 1837, 1954, 1887, 1854, 1874, 1477, 1921, 1426, 1989, 1833, 2022, 1835, 1892, 1890, 1859, 1883, 1975, 1871, 1989, 1874};
    int exec_time_arr[165]= {2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000};
    #endif

    #if _curseofwar_slice_sdl_
    int exec_time_arr[1002]= {159, 22, 18, 25, 35149, 33, 16, 16, 18, 25288, 34, 17, 18, 18, 25730, 32, 24, 20, 18, 31065, 59, 16, 17, 19, 25757, 32, 17, 15, 20, 25708, 33, 17, 15, 18, 30105, 43, 18, 17, 18, 31144, 33, 17, 15, 18, 25923, 59, 18, 18, 26, 25833, 34, 20, 16, 19, 25826, 34, 20, 15, 18, 31304, 36, 18, 17, 18, 26212, 34, 17, 19, 18, 25995, 35, 26, 23, 18, 25961, 34, 18, 39, 23, 31348, 32, 25, 18, 18, 26093, 33, 19, 18, 20, 25980, 34, 17, 16, 18, 26107, 34, 17, 14, 18, 31390, 33, 23, 15, 16, 26139, 35, 17, 15, 19, 26108, 33, 16, 19, 27, 26291, 57, 19, 16, 18, 31620, 34, 17, 15, 19, 26399, 40, 19, 16, 18, 26275, 33, 27, 15, 19, 26268, 34, 20, 18, 18, 31713, 34, 16, 15, 18, 26580, 33, 17, 19, 17, 26401, 33, 26, 22, 18, 26332, 35, 18, 24, 31, 32131, 35, 18, 16, 42, 26681, 34, 29, 15, 19, 26479, 33, 17, 24, 18, 26645, 34, 19, 18, 19, 32030, 34, 25, 15, 14, 26648, 42, 19, 15, 30, 26989, 32, 18, 17, 17, 26599, 35, 20, 15, 19, 32099, 34, 17, 15, 17, 26805, 34, 17, 15, 19, 26825, 32, 19, 15, 17, 26981, 43, 18, 15, 16, 32426, 33, 18, 17, 18, 26853, 34, 19, 15, 17, 26740, 35, 18, 15, 18, 26883, 45, 18, 15, 20, 32239, 34, 21, 15, 19, 26957, 47, 19, 18, 16, 26940, 34, 17, 23, 18, 26840, 34, 18, 18, 18, 32198, 40, 18, 16, 27, 27048, 34, 18, 16, 19, 26849, 34, 17, 16, 18, 26861, 35, 17, 15, 18, 32321, 34, 18, 16, 18, 27032, 34, 19, 17, 19, 26902, 42, 19, 16, 17, 27080, 33, 16, 16, 25, 32494, 34, 20, 15, 18, 27256, 33, 19, 15, 17, 27174, 34, 19, 15, 18, 27375, 41, 20, 16, 18, 32686, 34, 24, 15, 18, 27442, 34, 20, 17, 18, 27398, 34, 26, 17, 18, 27327, 33, 17, 18, 30, 32901, 41, 17, 16, 23, 27420, 34, 19, 24, 18, 27366, 42, 18, 15, 30, 27601, 33, 21, 15, 20, 32893, 34, 17, 15, 17, 28079, 34, 17, 17, 20, 27581, 33, 17, 16, 19, 27778, 33, 18, 16, 41, 32846, 34, 18, 15, 18, 27576, 33, 17, 23, 19, 27470, 34, 18, 15, 17, 27631, 33, 18, 16, 18, 32970, 36, 19, 25, 20, 27745, 34, 18, 16, 17, 27551, 34, 20, 16, 17, 27779, 35, 18, 16, 19, 32973, 45, 21, 16, 17, 27769, 33, 16, 16, 17, 27984, 34, 21, 16, 30, 27709, 34, 17, 24, 24, 33056, 33, 18, 16, 19, 28064, 34, 18, 14, 18, 27751, 35, 27, 17, 17, 27879, 34, 18, 16, 17, 33204, 32, 19, 15, 17, 27884, 33, 17, 16, 17, 27829, 33, 19, 16, 19, 27805, 34, 18, 15, 20, 33546, 35, 17, 16, 18, 29348, 42, 19, 16, 18, 27927, 40, 19, 16, 18, 27933, 33, 18, 17, 39, 33299, 32, 19, 16, 14, 28357, 33, 32, 15, 18, 28006, 33, 18, 25, 20, 28631, 35, 18, 15, 18, 33487, 33, 20, 16, 17, 28075, 47, 19, 17, 17, 28000, 33, 19, 19, 38, 28225, 35, 19, 16, 18, 33496, 46, 18, 15, 22, 28315, 34, 27, 16, 19, 28196, 43, 17, 16, 19, 28077, 35, 20, 17, 18, 33803, 35, 18, 27, 17, 28666, 35, 20, 14, 19, 28427, 44, 17, 18, 19, 28380, 44, 27, 16, 18, 33869, 35, 18, 16, 17, 28471, 34, 18, 17, 18, 28455, 46, 19, 16, 19, 28585, 40, 19, 17, 20, 33950, 34, 18, 16, 16, 28460, 34, 18, 15, 17, 28878, 36, 19, 15, 18, 28773, 32, 18, 16, 18, 34124, 35, 19, 16, 14, 28640, 35, 17, 16, 17, 28656, 41, 18, 15, 22, 28975, 36, 27, 16, 18, 34121, 34, 19, 17, 20, 28942, 34, 98, 17, 26, 28755, 46, 17, 15, 18, 28636, 34, 18, 17, 22, 34187, 42, 27, 16, 18, 28782, 33, 18, 15, 17, 28742, 35, 16, 16, 17, 28905, 32, 20, 16, 18, 34336, 33, 18, 16, 22, 28889, 34, 18, 31, 21, 28950, 34, 17, 16, 18, 28963, 34, 18, 16, 18, 34599, 34, 16, 25, 15, 28915, 37, 19, 16, 18, 28860, 50, 18, 16, 39, 29200, 35, 19, 16, 19, 34609, 32, 25, 17, 14, 29134, 35, 19, 16, 16, 29125, 34, 19, 18, 17, 29244, 34, 19, 15, 19, 34727, 35, 19, 15, 18, 29109, 35, 20, 15, 18, 29178, 34, 17, 16, 20, 29072, 42, 17, 17, 19, 34757, 78, 19, 15, 17, 29335, 34, 18, 18, 16, 29284, 35, 18, 16, 17, 29193, 35, 18, 16, 18, 34867, 34, 40, 15, 17, 29739, 34, 19, 18, 18, 29407, 34, 19, 16, 16, 29402, 43, 17, 19, 18, 35043, 33, 27, 15, 16, 29477, 33, 17, 15, 17, 29452, 47, 18, 16, 39, 29805, 34, 18, 15, 20, 35076, 36, 19, 16, 18, 29767, 59, 43, 16, 17, 29723, 33, 17, 16, 30, 29533, 35, 18, 16, 30, 35413, 34, 19, 15, 14, 29711, 34, 16, 18, 18, 29692, 44, 18, 16, 17, 29695, 34, 17, 15, 18, 35259, 43, 17, 14, 15, 29575, 34, 17, 37, 26, 29746, 35, 17, 15, 17, 29858, 34, 20, 16, 17, 35451, 75, 48, 16, 14, 29825, 35, 19, 16, 18, 29855, 33, 17, 16, 18, 29818, 35, 18, 15, 17, 35321, 34, 19, 15, 17, 29768, 36, 28, 18, 18, 29815, 34, 18, 15, 17, 29971, 34, 18, 17, 19, 35484, 59, 18, 17, 16, 29992, 59, 18, 18, 18, 29886, 36, 19, 25, 21, 30014, 57, 20, 15, 20, 35485, 36, 19, 16, 37, 29982, 34, 21, 16, 17, 30054, 34, 18, 21, 18, 29927, 33, 17, 17, 18, 36048, 40, 20, 16, 15, 30148, 34, 19, 17, 17, 30201, 35, 18, 17, 17, 30162, 47, 19, 17, 17, 35677, 34, 43, 17, 15, 30544, 71, 20, 16, 17, 30143, 34, 17, 17, 16, 30178, 43, 29, 26, 17, 35669, 34, 17};
    #endif

    #if _curseofwar_slice_
    int exec_time_arr[1002]= {159, 22, 18, 25, 35149, 33, 16, 16, 18, 25288, 34, 17, 18, 18, 25730, 32, 24, 20, 18, 31065, 59, 16, 17, 19, 25757, 32, 17, 15, 20, 25708, 33, 17, 15, 18, 30105, 43, 18, 17, 18, 31144, 33, 17, 15, 18, 25923, 59, 18, 18, 26, 25833, 34, 20, 16, 19, 25826, 34, 20, 15, 18, 31304, 36, 18, 17, 18, 26212, 34, 17, 19, 18, 25995, 35, 26, 23, 18, 25961, 34, 18, 39, 23, 31348, 32, 25, 18, 18, 26093, 33, 19, 18, 20, 25980, 34, 17, 16, 18, 26107, 34, 17, 14, 18, 31390, 33, 23, 15, 16, 26139, 35, 17, 15, 19, 26108, 33, 16, 19, 27, 26291, 57, 19, 16, 18, 31620, 34, 17, 15, 19, 26399, 40, 19, 16, 18, 26275, 33, 27, 15, 19, 26268, 34, 20, 18, 18, 31713, 34, 16, 15, 18, 26580, 33, 17, 19, 17, 26401, 33, 26, 22, 18, 26332, 35, 18, 24, 31, 32131, 35, 18, 16, 42, 26681, 34, 29, 15, 19, 26479, 33, 17, 24, 18, 26645, 34, 19, 18, 19, 32030, 34, 25, 15, 14, 26648, 42, 19, 15, 30, 26989, 32, 18, 17, 17, 26599, 35, 20, 15, 19, 32099, 34, 17, 15, 17, 26805, 34, 17, 15, 19, 26825, 32, 19, 15, 17, 26981, 43, 18, 15, 16, 32426, 33, 18, 17, 18, 26853, 34, 19, 15, 17, 26740, 35, 18, 15, 18, 26883, 45, 18, 15, 20, 32239, 34, 21, 15, 19, 26957, 47, 19, 18, 16, 26940, 34, 17, 23, 18, 26840, 34, 18, 18, 18, 32198, 40, 18, 16, 27, 27048, 34, 18, 16, 19, 26849, 34, 17, 16, 18, 26861, 35, 17, 15, 18, 32321, 34, 18, 16, 18, 27032, 34, 19, 17, 19, 26902, 42, 19, 16, 17, 27080, 33, 16, 16, 25, 32494, 34, 20, 15, 18, 27256, 33, 19, 15, 17, 27174, 34, 19, 15, 18, 27375, 41, 20, 16, 18, 32686, 34, 24, 15, 18, 27442, 34, 20, 17, 18, 27398, 34, 26, 17, 18, 27327, 33, 17, 18, 30, 32901, 41, 17, 16, 23, 27420, 34, 19, 24, 18, 27366, 42, 18, 15, 30, 27601, 33, 21, 15, 20, 32893, 34, 17, 15, 17, 28079, 34, 17, 17, 20, 27581, 33, 17, 16, 19, 27778, 33, 18, 16, 41, 32846, 34, 18, 15, 18, 27576, 33, 17, 23, 19, 27470, 34, 18, 15, 17, 27631, 33, 18, 16, 18, 32970, 36, 19, 25, 20, 27745, 34, 18, 16, 17, 27551, 34, 20, 16, 17, 27779, 35, 18, 16, 19, 32973, 45, 21, 16, 17, 27769, 33, 16, 16, 17, 27984, 34, 21, 16, 30, 27709, 34, 17, 24, 24, 33056, 33, 18, 16, 19, 28064, 34, 18, 14, 18, 27751, 35, 27, 17, 17, 27879, 34, 18, 16, 17, 33204, 32, 19, 15, 17, 27884, 33, 17, 16, 17, 27829, 33, 19, 16, 19, 27805, 34, 18, 15, 20, 33546, 35, 17, 16, 18, 29348, 42, 19, 16, 18, 27927, 40, 19, 16, 18, 27933, 33, 18, 17, 39, 33299, 32, 19, 16, 14, 28357, 33, 32, 15, 18, 28006, 33, 18, 25, 20, 28631, 35, 18, 15, 18, 33487, 33, 20, 16, 17, 28075, 47, 19, 17, 17, 28000, 33, 19, 19, 38, 28225, 35, 19, 16, 18, 33496, 46, 18, 15, 22, 28315, 34, 27, 16, 19, 28196, 43, 17, 16, 19, 28077, 35, 20, 17, 18, 33803, 35, 18, 27, 17, 28666, 35, 20, 14, 19, 28427, 44, 17, 18, 19, 28380, 44, 27, 16, 18, 33869, 35, 18, 16, 17, 28471, 34, 18, 17, 18, 28455, 46, 19, 16, 19, 28585, 40, 19, 17, 20, 33950, 34, 18, 16, 16, 28460, 34, 18, 15, 17, 28878, 36, 19, 15, 18, 28773, 32, 18, 16, 18, 34124, 35, 19, 16, 14, 28640, 35, 17, 16, 17, 28656, 41, 18, 15, 22, 28975, 36, 27, 16, 18, 34121, 34, 19, 17, 20, 28942, 34, 98, 17, 26, 28755, 46, 17, 15, 18, 28636, 34, 18, 17, 22, 34187, 42, 27, 16, 18, 28782, 33, 18, 15, 17, 28742, 35, 16, 16, 17, 28905, 32, 20, 16, 18, 34336, 33, 18, 16, 22, 28889, 34, 18, 31, 21, 28950, 34, 17, 16, 18, 28963, 34, 18, 16, 18, 34599, 34, 16, 25, 15, 28915, 37, 19, 16, 18, 28860, 50, 18, 16, 39, 29200, 35, 19, 16, 19, 34609, 32, 25, 17, 14, 29134, 35, 19, 16, 16, 29125, 34, 19, 18, 17, 29244, 34, 19, 15, 19, 34727, 35, 19, 15, 18, 29109, 35, 20, 15, 18, 29178, 34, 17, 16, 20, 29072, 42, 17, 17, 19, 34757, 78, 19, 15, 17, 29335, 34, 18, 18, 16, 29284, 35, 18, 16, 17, 29193, 35, 18, 16, 18, 34867, 34, 40, 15, 17, 29739, 34, 19, 18, 18, 29407, 34, 19, 16, 16, 29402, 43, 17, 19, 18, 35043, 33, 27, 15, 16, 29477, 33, 17, 15, 17, 29452, 47, 18, 16, 39, 29805, 34, 18, 15, 20, 35076, 36, 19, 16, 18, 29767, 59, 43, 16, 17, 29723, 33, 17, 16, 30, 29533, 35, 18, 16, 30, 35413, 34, 19, 15, 14, 29711, 34, 16, 18, 18, 29692, 44, 18, 16, 17, 29695, 34, 17, 15, 18, 35259, 43, 17, 14, 15, 29575, 34, 17, 37, 26, 29746, 35, 17, 15, 17, 29858, 34, 20, 16, 17, 35451, 75, 48, 16, 14, 29825, 35, 19, 16, 18, 29855, 33, 17, 16, 18, 29818, 35, 18, 15, 17, 35321, 34, 19, 15, 17, 29768, 36, 28, 18, 18, 29815, 34, 18, 15, 17, 29971, 34, 18, 17, 19, 35484, 59, 18, 17, 16, 29992, 59, 18, 18, 18, 29886, 36, 19, 25, 21, 30014, 57, 20, 15, 20, 35485, 36, 19, 16, 37, 29982, 34, 21, 16, 17, 30054, 34, 18, 21, 18, 29927, 33, 17, 17, 18, 36048, 40, 20, 16, 15, 30148, 34, 19, 17, 17, 30201, 35, 18, 17, 17, 30162, 47, 19, 17, 17, 35677, 34, 43, 17, 15, 30544, 71, 20, 16, 17, 30143, 34, 17, 17, 16, 30178, 43, 29, 26, 17, 35669, 34, 17};
    #endif

    #if _uzbl_
    int exec_time_arr[246]= {2589, 2479, 2393, 2328, 2350, 3593, 1677, 3225, 3773, 1208, 4427, 3422, 1460, 2501, 2487, 1632, 2920, 3036, 2408, 2157, 5637, 1305, 1188, 45, 5754, 3113, 3866, 3156, 4386, 2978, 1322, 3307, 3956, 4081, 3361, 3430, 1435, 2056, 3295, 1593, 1237, 2862, 7948, 4318, 3883, 3907, 4592, 4676, 4124, 4010, 3802, 3965, 3136, 2595, 2395, 55617, 2795, 768, 43536, 54, 3683, 1065, 5011, 3444, 4108, 4358, 3326, 1054, 3491, 2598, 2277, 2346, 702, 1328, 2542, 890, 655, 2490, 7863, 3388, 3732, 3502, 3399, 3934, 4142, 3744, 3615, 3425, 59261, 2618, 2713, 837, 45705, 56, 3756, 1088, 4166, 3417, 4434, 4390, 3257, 1105, 3687, 2576, 2404, 2306, 790, 1240, 2560, 962, 660, 3096, 2825, 2470, 4778, 4087, 3518, 3464, 3225, 60695, 2941, 1557, 2692, 2268, 3989, 1366, 1177, 5125, 3441, 1053, 3552, 4093, 3295, 3468, 1184, 2018, 4012, 1432, 1126, 3437, 2611, 3636, 2862, 61615, 4087, 1641, 2849, 2268, 3802, 2476, 1300, 7401, 3496, 1091, 3380, 3907, 3218, 3144, 1258, 2437, 2554, 876, 695, 2572, 2637, 2438, 2421, 59611, 3828, 1672, 2575, 2340, 4237, 3374, 4249, 4556, 3148, 1086, 3535, 3963, 3421, 3503, 1289, 2001, 3600, 1486, 1050, 3482, 2673, 2526, 2267, 47181, 4086, 2079, 2760, 2250, 2529, 2312, 1972, 2568, 2259, 2513, 2132, 1967, 2546, 2221, 2515, 2208, 2045, 2571, 2279, 2438, 2133, 1996, 2552, 2340, 2344, 2185, 1991, 2592, 2254, 2403, 2140, 397, 2565, 2316, 393, 2524, 2250, 396, 2560, 2351, 420, 2518, 2307, 407, 2549, 2462, 387, 2543, 2269, 2714, 2348, 2722, 2244, 1114};
    #endif

    #if _ldecode_
int exec_time_arr[300]= {25783, 25162, 24605, 11452, 22475, 24699, 24470, 11510, 22468, 25060, 24452, 12054, 22549, 25013, 24247, 11607, 22691, 24898, 24517, 11663, 22567, 24679, 24574, 11449, 22426, 24978, 24444, 11166, 22550, 25221, 24523, 11599, 22335, 24601, 24524, 11464, 22501, 24489, 24274, 11670, 22481, 24785, 24293, 11760, 22621, 24976, 24365, 11761, 22524, 24574, 24554, 11402, 22381, 24536, 24444, 11862, 22708, 24196, 24224, 11390, 22251, 24084, 24535, 11370, 22605, 24493, 24594, 11831, 22455, 24435, 24557, 11698, 22577, 24309, 24315, 11714, 22540, 24752, 24783, 11329, 22744, 24772, 24964, 11771, 22797, 24792, 24291, 11500, 22333, 25233, 24450, 11679, 22524, 24933, 24561, 11609, 22149, 24888, 24574, 11508, 22400, 24687, 24560, 11626, 22415, 24906, 24700, 11829, 22438, 24823, 24608, 11322, 22242, 25011, 24570, 11549, 22503, 25836, 24655, 11704, 22347, 25157, 24835, 11397, 22546, 24764, 24756, 11457, 22502, 25117, 24488, 11312, 22422, 25063, 24683, 11467, 22376, 25286, 24572, 11555, 22211, 25306, 24480, 11574, 23993, 25321, 24412, 11470, 22438, 24769, 24501, 11254, 22549, 24488, 24486, 11076, 21989, 23975, 24541, 11120, 22052, 26751, 26558, 11896, 22174, 24318, 24579, 11587, 22003, 24511, 24452, 11621, 22326, 24505, 24453, 11327, 21785, 24482, 24721, 11365, 21941, 24297, 24443, 11419, 21597, 24094, 24292, 11299, 21629, 24234, 24605, 11146, 21593, 24146, 24709, 11410, 21754, 24074, 24416, 11261, 21599, 24287, 24334, 11379, 21701, 24411, 24420, 11119, 21568, 24472, 24214, 11265, 21621, 24357, 24397, 11591, 21815, 24615, 24510, 11165, 21784, 24497, 24607, 11335, 21664, 24491, 24440, 11831, 22372, 24947, 24738, 11254, 22369, 24173, 24561, 11135, 21854, 24433, 24738, 11409, 21679, 24071, 25220, 11389, 22705, 25000, 25025, 11263, 25081, 26537, 24464, 11211, 22034, 24840, 24765, 11367, 21958, 24398, 24844, 11273, 22008, 24168, 24836, 11047, 22120, 24123, 24685, 11397, 22020, 24469, 24846, 10847, 21846, 24214, 24715, 11281, 22097, 24349, 24881, 12307, 24737, 24359, 24799, 12289, 25186, 25985, 26239, 11921, 23959, 24482, 24732, 11228, 22764, 24465, 24652, 11364, 22897, 24677, 24564, 11071};
    //int exec_time_arr[3000]= {36636, 35373, 32140, 13826, 27658, 28820, 28919, 13791, 27478, 29341, 28552, 13894, 27877, 33106, 31798, 15501, 32230, 29896, 30006, 15471, 32324, 32902, 29351, 13675, 27833, 30170, 29758, 13862, 28330, 30703, 24925, 12274, 25184, 29420, 29167, 13917, 29676, 30998, 29599, 14649, 28031, 29790, 29878, 14739, 27865, 29858, 29259, 14399, 29584, 30197, 30801, 14433, 32123, 32809, 30258, 14762, 29548, 30063, 30732, 14305, 29317, 30277, 30464, 14539, 29829, 30161, 30314, 14990, 24025, 25096, 26483, 14566, 28671, 29032, 29159, 14646, 28357, 29605, 29610, 13984, 28576, 29513, 29755, 14407, 28254, 29589, 29334, 14270, 28360, 30160, 29509, 14212, 27872, 29656, 29145, 14276, 27427, 29786, 29567, 14095, 28093, 29295, 29569, 14362, 27656, 29992, 29580, 14401, 28221, 29745, 29988, 11639, 22988, 25578, 29680, 14111, 28055, 29557, 25614, 12618, 23925, 30747, 30365, 15118, 27444, 30018, 30358, 14237, 28778, 31069, 30159, 13951, 29227, 30011, 29611, 13617, 28546, 30114, 29264, 14059, 27567, 30740, 29371, 14257, 27570, 30122, 29521, 14112, 28059, 30755, 29909, 13818, 31690, 29957, 30093, 13695, 27569, 29213, 31026, 14236, 29309, 30367, 30880, 14036, 29134, 30015, 30683, 14578, 29224, 30439, 30599, 14658, 29370, 31772, 30494, 14242, 29139, 30382, 30975, 14259, 27799, 29435, 29817, 14159, 27420, 29183, 29645, 14164, 27385, 29327, 29868, 14010, 27568, 29369, 30184, 14074, 31374, 32859, 33394, 15993, 25478, 27798, 27639, 15660, 27639, 30403, 31001, 14444, 28045, 30673, 30597, 14431, 31736, 29383, 33516, 15798, 27468, 30058, 29759, 13746, 27954, 29663, 29772, 14113, 27958, 29602, 29655, 14340, 27824, 29854, 29821, 13618, 29178, 25678, 26614, 14217, 31589, 33434, 30567, 14264, 28412, 29670, 30526, 13922, 28845, 30563, 30746, 14169, 28529, 30413, 31018, 14242, 28398, 30417, 30351, 14451, 29075, 30044, 30808, 14236, 28607, 30104, 31115, 14149, 28910, 30248, 30631, 15682, 31036, 33350, 26660, 11680, 22998, 29379, 31049, 14423, 31724, 25795, 26825, 14033, 37180, 29434, 30207, 13996, 29210, 30041, 30646, 14346, 29343, 30277, 31285, 14385, 30013, 30619, 31038, 14546, 29250, 30493, 30623, 15482, 26242, 31296, 17055, 9276, 22261, 30882, 16417, 8355, 22243, 31171, 16762, 8752, 22159, 30156, 15864, 8583, 20905, 31056, 16707, 10254, 23855, 31232, 17959, 10165, 23927, 31385, 18009, 8893, 22213, 28552, 17513, 8847, 22402, 25798, 14041, 7353, 22024, 28927, 16326, 9367, 21553, 28360, 16436, 9443, 21922, 30136, 16710, 9294, 23101, 30354, 16752, 9122, 22936, 29834, 16583, 9484, 22135, 30864, 16874, 9983, 26767, 28252, 16702, 8883, 22014, 29470, 16523, 9044, 21794, 29133, 16727, 8834, 21887, 28512, 16938, 9413, 22647, 29405, 16857, 7669, 20624, 29980, 17102, 9277, 22897, 31010, 17024, 9328, 23409, 29759, 18439, 9793, 24141, 29982, 17181, 9504, 23038, 29954, 17148, 9609, 23628, 29424, 16846, 9339, 22542, 30558, 16890, 9140, 22917, 28990, 16556, 9121, 23341, 28641, 16902, 9327, 23260, 25818, 14189, 7256, 25357, 30316, 14716, 7446, 19677, 24170, 20211, 11909, 25995, 29201, 16283, 9532, 21604, 28834, 16335, 9483, 21269, 27724, 15989, 10142, 22978, 30877, 17994, 9548, 23062, 27567, 15955, 8790, 20851, 33132, 17087, 8935, 21067, 28827, 16403, 8761, 20993, 27949, 15933, 8715, 20898, 29091, 15888, 8951, 21233, 28382, 15915, 8952, 20660, 28307, 15918, 8991, 20685, 28106, 15761, 8620, 21338, 27941, 16337, 8802, 23646, 28034, 15647, 9226, 21376, 28172, 15851, 8682, 21777, 28044, 16209, 9779, 21685, 25605, 15869, 7953, 19648, 31098, 15856, 9058, 20307, 28847, 15775, 8871, 20966, 27562, 16453, 10319, 22237, 28601, 16971, 9065, 21972, 28697, 16245, 9121, 21699, 28945, 16247, 9370, 21440, 27851, 16394, 9303, 21352, 28201, 16143, 9148, 22030, 27251, 15875, 9364, 21845, 25643, 14098, 7400, 19574, 27211, 15920, 9759, 24049, 27217, 15267, 9892, 25435, 29766, 17292, 8812, 22648, 27520, 15524, 9594, 22767, 27373, 15561, 9838, 23636, 27389, 15527, 9720, 23138, 26713, 15706, 9808, 23535, 22931, 15851, 9540, 23967, 27513, 16726, 9756, 24418, 26587, 16182, 9765, 24032, 23056, 14284, 7951, 19793, 27088, 16066, 7668, 23665, 24830, 17155, 10844, 27067, 29585, 16243, 10640, 22586, 26190, 14785, 9107, 23029, 26463, 14760, 9362, 29355, 32519, 26079, 15050, 23201, 27585, 23282, 16799, 21857, 29631, 23288, 16270, 21821, 29757, 23216, 16494, 21784, 33279, 24146, 15625, 23747, 27603, 23360, 15785, 23120, 27813, 23694, 15325, 23613, 27989, 23239, 15720, 23584, 28756, 23915, 15759, 23945, 28135, 23700, 15744, 23410, 27993, 24195, 15722, 23745, 28544, 24241, 15599, 22610, 28215, 24317, 15794, 23720, 30161, 24302, 15898, 23299, 28166, 23762, 15629, 22988, 24857, 21467, 15816, 23039, 29071, 23682, 16079, 22925, 31667, 24048, 15550, 22765, 28023, 24036, 15935, 22809, 28031, 24205, 15646, 22915, 27857, 24204, 15901, 22493, 28049, 24055, 15869, 22796, 27996, 23862, 15812, 22246, 28261, 23890, 15901, 23177, 28270, 24553, 15759, 23595, 26412, 20933, 14196, 19631, 30404, 24339, 16014, 22492, 28484, 23919, 15519, 22872, 28525, 23681, 16035, 23151, 28488, 23745, 17237, 24813, 31240, 27309, 15641, 23236, 28454, 23815, 15417, 23757, 28111, 23817, 15839, 23362, 28346, 23899, 17104, 22571, 28598, 24158, 16041, 23531, 28485, 24062, 15526, 23595, 28567, 24123, 15663, 19649, 24305, 21765, 15477, 23654, 28317, 24218, 12850, 19203, 23626, 28155, 22291, 32239, 39152, 34088, 20408, 31435, 39217, 33664, 19685, 26039, 33985, 30144, 18663, 26787, 34816, 34141, 19616, 29547, 31277, 34168, 21492, 29483, 36201, 27007, 17748, 24605, 31547, 27455, 17835, 25243, 32368, 27191, 20545, 29905, 37710, 34664, 19200, 25097, 32452, 29073, 18882, 26818, 39415, 34944, 21291, 31558, 33135, 28798, 19026, 29544, 33610, 29124, 19882, 26171, 33817, 28867, 22535, 24229, 39317, 29650, 18544, 26570, 33082, 29566, 19239, 27212, 31922, 29551, 19110, 27386, 41133, 28405, 22176, 32562, 39964, 35731, 18649, 26603, 39941, 36771, 20557, 27260, 33510, 29966, 18855, 27251, 32808, 28432, 18180, 27266, 33512, 30031, 18272, 28004, 34933, 22622, 15436, 22240, 34044, 28692, 18809, 23781, 39069, 26568, 18957, 26561, 34829, 29329, 19976, 26189, 32168, 28606, 21233, 28968, 26106, 22557, 14247, 25333, 37883, 29357, 28511, 34133, 38895, 37315, 17302, 24898, 33947, 27779, 17944, 24625, 31928, 30504, 17346, 24545, 31258, 28182, 17682, 32137, 38554, 34379, 21051, 36410, 25722, 18741, 8706, 28538, 24441, 17825, 7371, 22758, 18894, 19064, 7942, 31816, 24958, 14707, 6761, 23732, 20241, 21988, 10107, 40910, 28420, 20644, 8150, 28957, 27465, 19283, 6988, 22942, 20479, 18411, 9856, 30097, 25798, 20191, 9297, 35436, 31357, 19702, 8486, 33592, 29428, 19226, 8526, 36823, 30451, 22126, 10071, 30676, 26986, 21632, 9036, 29748, 24040, 19307, 8320, 31669, 29709, 20823, 8730, 37031, 25299, 20127, 9416, 34569, 26019, 19249, 7403, 23398, 19254, 15966, 7985, 29007, 23524, 19560, 8688, 32985, 23553, 18878, 8112, 27209, 23662, 19805, 8245, 30493, 24215, 18090, 8855, 28868, 24524, 15480, 6600, 25780, 20394, 19853, 8884, 31482, 24908, 18607, 8533, 28150, 22656, 16894, 7825, 27201, 22715, 16348, 8230, 27267, 22337, 16887, 8007, 26663, 22894, 16845, 7977, 27183, 22607, 17108, 7906, 27122, 21181, 15520, 6501, 23959, 23440, 16805, 7880, 23791, 22073, 15002, 6529, 28718, 23751, 26366, 8876, 33152, 27844, 16766, 7722, 28142, 24408, 19020, 9131, 32592, 27607, 19322, 8500, 32073, 26843, 17742, 8031, 28325, 24816, 17370, 8182, 28341, 25228, 18109, 7580, 27240, 24540, 18105, 8170, 28146, 24549, 18250, 7440, 26757, 23734, 18785, 7785, 27229, 25240, 18391, 8241, 26832, 24005, 15802, 6702, 27620, 24646, 18529, 7971, 26912, 20787, 15499, 7189, 27727, 24556, 18134, 9038, 27984, 24759, 17491, 8031, 26841, 25416, 20252, 7996, 28121, 24906, 17839, 7438, 26780, 25646, 17831, 8128, 26894, 24988, 17281, 6645, 23713, 22006, 19337, 7625, 28675, 25820, 18969, 8088, 28156, 25952, 18508, 8181, 27762, 28423, 18104, 8340, 31365, 28081, 18163, 8471, 28044, 24861, 18543, 7544, 27608, 24909, 17973, 8011, 28018, 25259, 17886, 8309, 27584, 25799, 17759, 8122, 27795, 25924, 18506, 8004, 27895, 26109, 15854, 6491, 24551, 25797, 17698, 7891, 28160, 25677, 18617, 8097, 28008, 26421, 18574, 7909, 27257, 26137, 17262, 7951, 26593, 25596, 16453, 8283, 27177, 22030, 13079, 7501, 26327, 26037, 14756, 8337, 27042, 26102, 14383, 7775, 28347, 27246, 15253, 8038, 28572, 29014, 16365, 7880, 28573, 29059, 14677, 6622, 29908, 19129, 23385, 11854, 28535, 17823, 22801, 12221, 23428, 15572, 19249, 10357, 26716, 17849, 23204, 13271, 29129, 20236, 25164, 12248, 29755, 19868, 24426, 13695, 29494, 17594, 23122, 11790, 28507, 18029, 22964, 11740, 28713, 18218, 22915, 12201, 28626, 18162, 23001, 12070, 28437, 18190, 23347, 11976, 29267, 18187, 23156, 12433, 29816, 18514, 23862, 12423, 24855, 15675, 19012, 10152, 28024, 17821, 24635, 11644, 26378, 18458, 26941, 12171, 29750, 21944, 22501, 12258, 29679, 18406, 22715, 13064, 32046, 20548, 24919, 13594, 32827, 20402, 25006, 13537, 32689, 18806, 21637, 12571, 27893, 19240, 22211, 12108, 28688, 19430, 23692, 12419, 32474, 20653, 24756, 14463, 26104, 16150, 20807, 11838, 28287, 18574, 22105, 12423, 28222, 18765, 22729, 12082, 28239, 17659, 22591, 12200, 28200, 18095, 24023, 12524, 29582, 18703, 23846, 14031, 32841, 18322, 22374, 12063, 32968, 20589, 24967, 13888, 28390, 18552, 22744, 12424, 29204, 18364, 22585, 12088, 23526, 18901, 25211, 13739, 30125, 18602, 25448, 12793, 29860, 18877, 23979, 12333, 30537, 19042, 23802, 12156, 29059, 18021, 23867, 12663, 28430, 18252, 22798, 12224, 32712, 20787, 23137, 12656, 28332, 18738, 23147, 12213, 29112, 18165, 23384, 12822, 24130, 15975, 20153, 12192, 29241, 18132, 23775, 12068, 28953, 18444, 23251, 12131, 30210, 18778, 23790, 12672, 29640, 18447, 23201, 12093, 28949, 18143, 22662, 12108, 28854, 18590, 22865, 11968, 28270, 18145, 22676, 12023, 28653, 18301, 22333, 12100, 28758, 18344, 23023, 12208, 28567, 18390, 23421, 12160, 28382, 16117, 19391, 10339, 28212, 18122, 25942, 14913, 31582, 16368, 19276, 10401, 27972, 18524, 24344, 12648, 29294, 18786, 23965, 12755, 34020, 21780, 25243, 13534, 32039, 21746, 26478, 13426, 32123, 21608, 24626, 13569, 31860, 28132, 25061, 13677, 29004, 19921, 22801, 13033, 28183, 18588, 20284, 10177, 23199, 18544, 22952, 11840, 28139, 18776, 26191, 13587, 32557, 20331, 25193, 14027, 29423, 19132, 23108, 12135, 28407, 18434, 23865, 12168, 29916, 19079, 26910, 11094, 26232, 18076, 25917, 12026, 29914, 19102, 24065, 12238, 28618, 18612, 23356, 12264, 29315, 18530, 26680, 13500, 35270, 32299, 23649, 13600, 27888, 27851, 21129, 12228, 31478, 32150, 25014, 13566, 34557, 31336, 24239, 14086, 32343, 31111, 24006, 13171, 33568, 31438, 24461, 13267, 33622, 32077, 24249, 14062, 32986, 32223, 24905, 13994, 33713, 32199, 24973, 14324, 33542, 31847, 25026, 14456, 33746, 32199, 24958, 14396, 34152, 32346, 24458, 14359, 32576, 32802, 21433, 11920, 27199, 29339, 24484, 13309, 32905, 31924, 24573, 13022, 33537, 32280, 24703, 12951, 33476, 27876, 20916, 10710, 29955, 38059, 25292, 13142, 33224, 33148, 24315, 13156, 36887, 36560, 27407, 14427, 33130, 32769, 24834, 13212, 34108, 32820, 24323, 11129, 28052, 27827, 24406, 13004, 34054, 32588, 23745, 13471, 34038, 33647, 25371, 12790, 34680, 37797, 24945, 13161, 38357, 36257, 29011, 13011, 34131, 33244, 25768, 13645, 34261, 33361, 25734, 13883, 34792, 32813, 26131, 14002, 34350, 31941, 25386, 13518, 34108, 32697, 25205, 14171, 28766, 27579, 26295, 13872, 36772, 36909, 29000, 12525, 31077, 31447, 25385, 13490, 34732, 32307, 25771, 13685, 34139, 32174, 25728, 13681, 34550, 31953, 25595, 13606, 34239, 31728, 26036, 13482, 33411, 31847, 25564, 14519, 30875, 30548, 23479, 14889, 33577, 32033, 21861, 12430, 29867, 32223, 25904, 13963, 33965, 32785, 25746, 16629, 32942, 32203, 26145, 14307, 34146, 31391, 24806, 13759, 33565, 31815, 25279, 13338, 34720, 32048, 25095, 13669, 34309, 31732, 25595, 13638, 33360, 31608, 25599, 13399, 33115, 27446, 22610, 14489, 33128, 27463, 21395, 12174, 30071, 34040, 27633, 13989, 34066, 32244, 30548, 14479, 32423, 32974, 25638, 14931, 34753, 33430, 32864, 14290, 33881, 32957, 26237, 14269, 33160, 32082, 25124, 14193, 33639, 32395, 25324, 14137, 34033, 31917, 25203, 15526, 36436, 35774, 26347, 12518, 28163, 27992, 25212, 14298, 33982, 32220, 25484, 14369, 34500, 33118, 26296, 14284, 34505, 32363, 26007, 12398, 28695, 32316, 25839, 14595, 34069, 33398, 26258, 14729, 34252, 33011, 26640, 14639, 34123, 32836, 26379, 14959, 33947, 33185, 25597, 14511, 31214, 27154, 22722, 14505, 36645, 35380, 28960, 15632, 35169, 35552, 27804, 15818, 34750, 29538, 21382, 12443, 26747, 31527, 25155, 14098, 36397, 27048, 17140, 7051, 29888, 29726, 19592, 7300, 28294, 25668, 19445, 9739, 35034, 30758, 17256, 7600, 29888, 30165, 18434, 8908, 36250, 26901, 17313, 7667, 33589, 31088, 18642, 9826, 32433, 31039, 18692, 10008, 31607, 30115, 19150, 9498, 31972, 30317, 19067, 9216, 31956, 45167, 21476, 10158, 36815, 33339, 21208, 10647, 36733, 30255, 19579, 9355, 33527, 30317, 19367, 8596, 32223, 30041, 18593, 9493, 31883, 30398, 18932, 8796, 32066, 30356, 18908, 9241, 32319, 30824, 18320, 9111, 31859, 30474, 18872, 8682, 32219, 30694, 18728, 9224, 32067, 30500, 18959, 10843, 33886, 30223, 19675, 9534, 31846, 30851, 19268, 9201, 32608, 30237, 18428, 9792, 38197, 34486, 21443, 10376, 37544, 34077, 20878, 10118, 35380, 34489, 20571, 9287, 37586, 34076, 21016, 10607, 37169, 30826, 18634, 9393, 33235, 30509, 18124, 9508, 32140, 29580, 18837, 9141, 31705, 30013, 19107, 9142, 33368, 30508, 18768, 8419, 27555, 26226, 16885, 9444, 32398, 29730, 18694, 9492, 32101, 30176, 18616, 9217, 32723, 30600, 18757, 9066, 33781, 31283, 19301, 8800, 32622, 31246, 18751, 9179, 32438, 30985, 19322, 9569, 33192, 32215, 19382, 8812, 32110, 31303, 19002, 9934, 32037, 31384, 18685, 7860, 29506, 29626, 19683, 10442, 35978, 32365, 19721, 9394, 34758, 32254, 19576, 9377, 33157, 32802, 20301, 8927, 31521, 31670, 19301, 9223, 31606, 35747, 21822, 10113, 33694, 33136, 19438, 7692, 27570, 27984, 18884, 9405, 29982, 27604, 19168, 9829, 34610, 35900, 20954, 10495, 33956, 31169, 19006, 9339, 32234, 31878, 19530, 8484, 32081, 31524, 18820, 9167, 37061, 35107, 21498, 10438, 36954, 34807, 21084, 9989, 37053, 35227, 19245, 9610, 33762, 26867, 17652, 8185, 34087, 31890, 19460, 9167, 34927, 31674, 19759, 9197, 32762, 31819, 19200, 9309, 32934, 31694, 18786, 9131, 33380, 31774, 19360, 10220, 42940, 30812, 19170, 9115, 31837, 30417, 19072, 9291, 32168, 30593, 18736, 8650, 32060, 31220, 19011, 8960, 33172, 31553, 19007, 9228, 34252, 27205, 18871, 9117, 35092, 30978, 16682, 7329, 28785, 31968, 22664, 9082, 34283, 32088, 20961, 8957, 34921, 31831, 19443, 9605, 32797, 34122, 28507, 15591, 31840, 31482, 28124, 18256, 34737, 35934, 28249, 17783, 30128, 31538, 28584, 17580, 31890, 31718, 28344, 17658, 31085, 30887, 28819, 17690, 31144, 31077, 28646, 17479, 30572, 32325, 28935, 17678, 31151, 33457, 25710, 15228, 30799, 36217, 30264, 15933, 28020, 31876, 32838, 19567, 29622, 32392, 29298, 18056, 29387, 32228, 29523, 18002, 29852, 32471, 29211, 17624, 30271, 32174, 29398, 17744, 31413, 31743, 29284, 17403, 30414, 32199, 29554, 18052, 30035, 33252, 25143, 15737, 25527, 34547, 25301, 19157, 33178, 28341, 29897, 18383, 30757, 36420, 34039, 20030, 33705, 36473, 32950, 21000, 33305, 36909, 33050, 19215, 34234, 37084, 29170, 17895, 30885, 32785, 28995, 17831, 29624, 32425, 29026, 17512, 30383, 31787, 29298, 18579, 30219, 32686, 29556, 18531, 30511, 33045, 30253, 17611, 31408, 32977, 29968, 17553, 30650, 32546, 29067, 17199, 30780, 33021, 24718, 15797, 29030, 32070, 28255, 17488, 27913, 36879, 36452, 19543, 35124, 36051, 30076, 18045, 29768, 39198, 29981, 17440, 31844, 32902, 30237, 19831, 33867, 33609, 29096, 17214, 30865, 32460, 29144, 17313, 31139, 32268, 32711, 17164, 31451, 32615, 29501, 18829, 29536, 31841, 29168, 17361, 33105, 36739, 32792, 20356, 33152, 35727, 32774, 17454, 30564, 32023, 28696, 17882, 30947, 33485, 29240, 17986, 30935, 33119, 29657, 15383, 26604, 28346, 29782, 18076, 32224, 33683, 25821, 14904, 26513, 33056, 29588, 18755, 30315, 33166, 29445, 18448, 30922, 32933, 29553, 18694, 30881, 33925, 29397, 18456, 29779, 32317, 29581, 15927, 26170, 29045, 28906, 18734, 26605, 27826, 24880, 19520, 30531, 32252, 29230, 17288, 30440, 31774, 28683, 18900, 30152, 31936, 29170, 18110, 30903, 32199, 28883, 17976, 29259, 36422, 32083, 20200, 33498, 36447, 32378, 17517, 31004, 32687, 26941, 15604, 25917, 27557, 26590, 15783, 25873, 32410, 29460, 17962, 25567, 27932, 26389, 18608, 30382, 31984, 29590, 18959, 30628, 32525, 29264, 18189, 30620, 33005, 29763, 19412, 29652, 32995, 29980, 18096, 30574, 40955, 29912, 18313, 32895, 37245, 32385, 18390, 30526, 32599, 29819, 18006, 30334, 32241, 29539, 18148, 33803, 37303, 33382, 19916, 29282, 35923, 30833, 15952, 28148, 33343, 30971, 16397, 27301, 33172, 36302, 16771, 27429, 33235, 31317, 16386, 28785, 33540, 31759, 16695, 23545, 33806, 31564, 16723, 28652, 33773, 32553, 17074, 28556, 32952, 31238, 16365, 28053, 33044, 31151, 16381, 27595, 28480, 26637, 14621, 26557, 33051, 31364, 16335, 31483, 37084, 35328, 18621, 29323, 37094, 35755, 18449, 28700, 37405, 35356, 18196, 24916, 31424, 29172, 18945, 26342, 33567, 31662, 16420, 27232, 33726, 30907, 16315, 28187, 30489, 26713, 14478, 25968, 33415, 31791, 16423, 28690, 33276, 31328, 16490, 28121, 33561, 32549, 14801, 22629, 29116, 32051, 17090, 28753, 34192, 32257, 16896, 28812, 34146, 31392, 16877, 28475, 33742, 31897, 16542, 28790, 34033, 32620, 17159, 24231, 28573, 28415, 16920, 29833, 34088, 27760, 16572, 23358, 33661, 31474, 16724, 24238, 33649, 27007, 14824, 23231, 36628, 35393, 21250, 25253, 32513, 30926, 18506, 32163, 35769, 30726, 17259, 28525, 34955, 34660, 18863, 27732, 34683, 32044, 17693, 28724, 34787, 32707, 17055, 28778, 34193, 31744, 17078, 24956, 33759, 32075, 16858, 28615, 33991, 32157, 16861, 31372, 36737, 35198, 18541, 30021, 37474, 34880, 18710, 31267, 37377, 35059, 18658, 30079, 37709, 35206, 17808, 27197, 37986, 35137, 18546, 30164, 37606, 37008, 19168, 30896, 37316, 30916, 16716, 31438, 37914, 35214, 18711, 27466, 33406, 31767, 16809, 28599, 33457, 31769, 16891, 27218, 34131, 31617, 17001, 28230, 33824, 32109, 16755, 30391, 37399, 34814, 18141, 27509, 31004, 26599, 17155, 26311, 34295, 31399, 16532, 28200, 34002, 32016, 16570, 28808, 34450, 31847, 16564, 28712, 35058, 31843, 17167, 28171, 33613, 31305, 16516, 28274, 33619, 31751, 16611, 31462, 37208, 31586, 16589, 31514, 36920, 35259, 18085, 29371, 31023, 29566, 14128, 32212, 36895, 35499, 18271, 29837, 34055, 32107, 16881, 27726, 33664, 31422, 16638, 31037, 37970, 35305, 19145, 31963, 37625, 31437, 16526, 31614, 37943, 26661, 14990, 22909, 33378, 31089, 16701, 22764, 28648, 26350, 20299, 29702, 33840, 32242, 17252, 29298, 33863, 31644, 17467, 28415, 33533, 31908, 16718, 28206, 33972, 32077, 18156, 30138, 36540, 31947, 17009, 39204, 37432, 23532, 13343, 31251, 33146, 23260, 13878, 31356, 32517, 23609, 13282, 30596, 31707, 23215, 12979, 30445, 32210, 23698, 12782, 32076, 32341, 23849, 12557, 31891, 32463, 22651, 13123, 30602, 34357, 23480, 14367, 35059, 36403, 27865, 14238, 35672, 37292, 26309, 14497, 31709, 32574, 23522, 13041, 32700, 32330, 23626, 13158, 32610, 32720, 23774, 12840, 32756, 33206, 23338, 12806, 32144, 32743, 23674, 12178, 32852, 33072, 23973, 12824, 32942, 32309, 23223, 12648, 33211, 28046, 20776, 10733, 29412, 32983, 23780, 12936, 32342, 32392, 27055, 14406, 35233, 36596, 27501, 14416, 35354, 36672, 26549, 14868, 35148, 36501, 27086, 14586, 30673, 32821, 23282, 12929, 32627, 33024, 24184, 13585, 35282, 33347, 23849, 13802, 34914, 36622, 26029, 13834, 35423, 36496, 27061, 14642, 30424, 27999, 21320, 11020, 31575, 33156, 24897, 13525, 32095, 33339, 23858, 12461, 31966, 33553, 24114, 12409, 32449, 32779, 23932, 12199, 32202, 32540, 24390, 12430, 32957, 34108, 24806, 13266, 32791, 33901, 24670, 12538, 31679, 33132, 24132, 12385, 30006, 28637, 21425, 10952, 32455, 34668, 25640, 11303, 26730, 29149, 24267, 12918, 33467, 34213, 24378, 13247, 31373, 34143, 24085, 12919, 32382, 33327, 23553, 12711, 32072, 33269, 23881, 12812, 32395, 32807, 24025, 12910, 31828, 33240, 23713, 12989, 32197, 33169, 24178, 14229, 32006, 32794, 23770, 13453, 32047, 32650, 23903, 13375, 33194, 34173, 24241, 13728, 37758, 36740, 26784, 14982, 32358, 32827, 23660, 13135, 32514, 38964, 23408, 12599, 32493, 33525, 22779, 12493, 32579, 32803, 23134, 13147, 32931, 33534, 24164, 12288, 33133, 34001, 24011, 12639, 36213, 37284, 23660, 12005, 31213, 32882, 23671, 12158, 32737, 33059, 23496, 12288, 32149, 33187, 23598, 12421, 35602, 37018, 27486, 14383, 34680, 37497, 27496, 12643, 31561, 33609, 24794, 13407, 26726, 28860, 23193, 13772, 34623, 38311, 26973, 15322, 34641, 37110, 27162, 14269, 35151, 37925, 26959, 14626, 31696, 33166, 23869, 12502, 32280, 33341, 24086, 13465, 33282, 34052, 24508, 13190, 31764, 33074, 24436, 12485, 32105, 32739, 24140, 12942, 33129, 28874, 21325, 12551, 35005, 37038, 28657, 15046};
    #endif   

#endif


#endif
