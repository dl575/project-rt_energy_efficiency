/* This file includes common functions which 
some of all benchmarks use.
*/

#ifndef __MY_COMMON_H__
#define __MY_COMMON_H__

#include "timing.h"
//constant
#define MILLION 1000000L

//manually set below
#define CORE 1 //0:LITTLE, 1:big

#define PREDICT_EN 1 //0:prediction off, 1:prediction on
#define DELAY_EN 1 //0:delay off, 1:delay on
#define OVERHEAD_EN 0 //1:measure dvfs, slice timing

#define GET_PREDICT 0 //to get prediction equation
#define GET_OVERHEAD 0 // to get execution deadline
#define GET_OVERHEAD 0 //to get overhead deadline
#define DEBUG_EN 0 //debug information print on/off

//always set this as 1 on ODROID
#define DVFS_EN 1 //1:change dvfs, 0:don't change dvfs (e.g., not running on ODROID)

//automatically set
#define MAX_FREQ ((CORE)?(2000000):(1400000))



FILE *fp_max_freq; //File pointer scaling_max_freq

void fopen_all(void){
#if DVFS_EN
    #if CORE //big
        if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq", "w"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
        }
    #else //LITTLE
        if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "w"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
    }
    #endif
#endif
    return;
}

void fclose_all(void){
#if DVFS_EN
   fclose(fp_max_freq);
#endif
    return;
}

void set_freq(float predicted_exec_time, int slice_time, int deadline_time, int avg_dvfs_time){
#if DVFS_EN
    int predicted_freq = MAX_FREQ;
       
    //calculate predicted freq and round up by adding 99999
    predicted_freq = predicted_exec_time * MAX_FREQ / (deadline_time - slice_time - avg_dvfs_time) + 99999;
    //if less then 200000, just set it minimum (200000)
    predicted_freq = (predicted_freq < 200000 || predicted_exec_time <= 0)?(200000):(predicted_freq);
    //set maximum frequency, because performance governor always use maximum freq.
    fprintf(fp_max_freq, "%d", predicted_freq);
    fflush(fp_max_freq);
#endif

    return;
}

void set_freq_uzbl(float predicted_exec_time, int slice_time, int deadline_time, int avg_dvfs_time){
#if DVFS_EN
    #if CORE //big
        if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq", "w"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
        }
    #else //LITTLE
        if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "w"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
    }
    #endif
#endif
    int predicted_freq = MAX_FREQ;
       
    //calculate predicted freq and round up by adding 99999
    predicted_freq = predicted_exec_time * MAX_FREQ / (deadline_time - slice_time - avg_dvfs_time) + 99999;
    //if less then 200000, just set it minimum (200000)
    predicted_freq = (predicted_freq < 200000 || predicted_exec_time <= 0)?(200000):(predicted_freq);
    //set maximum frequency, because performance governor always use maximum freq.
    fprintf(fp_max_freq, "%d", predicted_freq);
#if DVFS_EN
    fclose(fp_max_freq);
#endif
    return;
}


void fprint_freq(void){
#if DVFS_EN
    FILE *fp_power; //File pointer of power of A7 (LITTLE) core or A15 (big) core power sensor file
    FILE *fp_freq; //File pointer of freq of A7 (LITTLE) core or A15 (big) core power sensor file
    int khz; //Value (khz) at start point.

    FILE *time_file;
    time_file = fopen("times.txt", "a");
    #if CORE //big
        if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq", "r"))){
            printf("ERROR : FILE READ FAILED\n");
            return;
        }
        fscanf(fp_freq, "%d", &khz);
        fprintf(time_file, "big core freq : %dkhz\n", khz);  
    #else //LITTLE
        if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r"))){
            printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
            return;
        }
        fscanf(fp_freq, "%d", &khz);
        fprintf(time_file, "little core freq : %dkhz\n", khz);  
    #endif
        fclose(fp_freq);
    fclose(time_file); 
    return;
#endif
}

void print_freq(void){
    FILE *fp_power; //File pointer of power of A7 (LITTLE) core or A15 (big) core power sensor file
    FILE *fp_freq; //File pointer of freq of A7 (LITTLE) core or A15 (big) core power sensor file
    int khz; //Value (khz) at start point.

#if CORE //big
    if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return;
    }
    fscanf(fp_freq, "%d", &khz);
    printf("big core freq : %dkhz\n", khz);  
#else //LITTLE
    if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
    }
    fscanf(fp_freq, "%d", &khz);
    printf("little core freq : %dkhz\n", khz);  
#endif
    fclose(fp_freq);
    return;
}



void fprint_deadline(int deadline_time){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "============ deadline time : %d us ===========\n", deadline_time);//TJSong
    fclose(time_file);
}

void print_deadline(int deadline_time){
    printf("============ deadline time : %d us ===========\n", deadline_time);//TJSong
}

void fprint_predicted_time(float predicted_exec_time){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "predicted time = %f\n", predicted_exec_time);
    fclose(time_file);
}

void print_predicted_time(float predicted_exec_time){
    printf("predicted time = %f\n", predicted_exec_time);
}

void fprint_exec_time(int exec_time){
    static int instance_number = 0;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "time %d = %d us\n", instance_number, exec_time);
    instance_number++;
    fclose(time_file);
}

void print_exec_time(int exec_time){
    static int instance_number = 0;
    printf("time %d = %d us\n", instance_number, exec_time);
    instance_number++;
}

void fprint_total_time(int exec_time){
    static int instance_number = 0;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "total_time %d = %d us\n", instance_number, exec_time);
    instance_number++;
    fclose(time_file);
}

void print_total_time(int exec_time){
    static int instance_number = 0;
    printf("total_time %d = %d us\n", instance_number, exec_time);
    instance_number++;
}



#endif
