#ifndef __TIMING_H__
#define __TIMING_H__

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "deadline.h"
#include "my_common.h"

extern struct timeval start, end, moment;
extern int client_join = 0;

//Define function names
//all benchmarks use below common timing functions
void start_timing();
void end_timing();
int exec_timing();
void init_time_file();
void my_usleep(unsigned long us);

//These functions are defined differently by F_PRINT
void print_array(int *array, int array_len);
void print_timing();
void moment_timing_print();
int print_slice_timing();
int print_dvfs_timing();
void print_deadline(int deadline_time);
void print_predicted_time(float predicted_exec_time);
void print_exec_time(int exec_time);
void print_total_time(int exec_time);
void print_delay_time(int pre_delay_time, int delay_time);
void print_current_core(int current_core);

void print_start_temperature();
void print_end_temperature();
void print_file(FILE *file);

//Implement actual function body
/*
 * Start of section to time.
 */
void start_timing() {
  gettimeofday(&start, NULL);
}

/*
 * Stop timing, record end time.
 */
void end_timing() {
  gettimeofday(&end, NULL);
}
/*
 * Return excute time
 */
int exec_timing() {
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}
/*
 * Create new times.txt file
 */
void init_time_file() {
  FILE *time_file;
  time_file = fopen("times.txt", "w");
  fclose(time_file);
}
/*
 * Original usleep function doesn't work due to interrupt issues.
 * This version works with this issues.
 * BUT, we need to figure it out why.
 */
void my_usleep(unsigned long us)
{
    struct timespec req={0};
    time_t sec=(int)(us/1000000);
    us=us-(sec*1000000);
    req.tv_sec=sec;
    req.tv_nsec=us*1000L;
    while(nanosleep(&req,&req)==-1)
        continue;
}

#if !F_PRINT //just use printf
/*
 * Write the passed integer array to the timing file.
 */
void print_array(int *array, int array_len) {
  printf("loop counter = (");
  int i;
  for (i = 0; i < array_len; i++) {
      printf("%d, ", array[i]);
  }
  printf(")\n");
  fclose(time_file);
}
/*
 * Print timing information to stdout.
 */
void print_timing() {
  static int instance_number = 0;
  printf("time %d = %d us\n", instance_number, 
    (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
  instance_number++;
}
/*
 * Moment timing, print moment.
 */
void moment_timing_print(int start_end) {
    gettimeofday(&moment, NULL);
    
    if(start_end == 0){//moment_start
        printf("moment_start_0 : %llu us\n", (unsigned long long int)moment.tv_sec * MILLION + (unsigned long long int)moment.tv_usec);
    }else if(start_end == 1){//moment_start
        printf("moment_start_1 : %llu us\n", (unsigned long long int)moment.tv_sec * MILLION + (unsigned long long int)moment.tv_usec);
    }else{//moment_end
        printf("moment_end : %llu us\n", (unsigned long long int)moment.tv_sec * MILLION + (unsigned long long int)moment.tv_usec);
    }
}
/*
 * Print slicing timing information to stdout.
 */
int print_slice_timing() {
  static int instance_number = 0;
  printf("time_slice %d = %d us\n", instance_number, 
    (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
  instance_number++;
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}
/*
 * Print dvfs timing information to stdout.
 */
int print_dvfs_timing() {
  static int instance_number = 0;
  printf("time_dvfs %d = %d us\n", instance_number, 
    (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
  instance_number++;
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}
/*
 * Print deadline information to stdout.
 */
void print_deadline(int deadline_time){
    printf("============ deadline time : %d us ===========\n", deadline_time);
}
void print_predicted_time(float predicted_exec_time){
    printf("predicted time = %f\n", predicted_exec_time);
}
void print_exec_time(int exec_time){
    static int instance_number = 0;
    printf("time %d = %d us\n", instance_number, exec_time);
    instance_number++;
}
void print_total_time(int exec_time){
    static int instance_number = 0;
    printf("time_total %d = %d us\n", instance_number, exec_time);
    instance_number++;
}
void print_delay_time(int pre_delay_time, int delay_time){
    static int instance_number = 0;
    printf("delay should be %d = %d us\n", instance_number, pre_delay_time);
    printf("actual dealy is %d = %d us\n", instance_number, delay_time);
    instance_number++;
}
void print_current_core(int current_core){
    printf("current_core : %d\n", current_core);
}
/*
 * Print start temperature
 */
void print_start_temperature() {
    static int instance_number = 0;
    FILE *fp_tmu; //File pointer of TMU file
    if(NULL == (fp_tmu = fopen("/sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp", "r"))){
        printf("TMU Read Error\n");
        return -1;
    }
    printf("start_temperature %d\n", instance_number);
    print_file(fp_tmu);
    instance_number++;
}
/*
 * Print end temperature
 */
void print_end_temperature() {
    static int instance_number = 0;
    FILE *fp_tmu; //File pointer of TMU file
    if(NULL == (fp_tmu = fopen("/sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp", "r"))){
        printf("TMU Read Error\n");
        return -1;
    }
    printf("end_temperature %d\n", instance_number);
    print_file(fp_tmu);
    instance_number++;
}
/*
 * Print file
 */
void print_file(FILE *file){
    int c;
    if(file){
        while((c = getc(file)) != EOF){
            putchar(c);
        }
        fclose(file);
    }
    return;
}

#elif F_PRINT //some benchmarks use file "times.txt" to print log 
/*
 * Write the passed integer array to the timing file.
 */
void print_array(int *array, int array_len) {
  // Open file for writing
  FILE *time_file;
  time_file = fopen("times.txt", "a");
  if (time_file == NULL) {
    printf("Error opening times.txt!\n");
    exit(1);
  }

  // Write time
  fprintf(time_file, "loop counter = (");
  int i;
  for (i = 0; i < array_len; i++) {
      fprintf(time_file, "%d, ", array[i]);
  }
  fprintf(time_file, ")\n");
  fclose(time_file);
}
/*
 * Print timing information to stdout.
 */
void print_timing() {
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    static int instance_number = 0;
    fprintf(time_file, "time %d = %d us\n", instance_number, 
        (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
    instance_number++;
    fclose(time_file);
}

/*
 * Moment timing, fprint moment.
 */
void moment_timing_print(int start_end) {
    gettimeofday(&moment, NULL);
    
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    if(start_end == 0){//moment_start
        fprintf(time_file, "moment_start_0 : %llu us\n", (unsigned long long int)moment.tv_sec * MILLION + (unsigned long long int)moment.tv_usec);
    }else if(start_end == 1){//moment_start
        fprintf(time_file, "moment_start_1 : %llu us\n", (unsigned long long int)moment.tv_sec * MILLION + (unsigned long long int)moment.tv_usec);
    }else{//moment_end
        fprintf(time_file, "moment_end : %llu us\n", (unsigned long long int)moment.tv_sec * MILLION + (unsigned long long int)moment.tv_usec);
    }
    fclose(time_file);
}
/*
 * Write slicing timing information to times.txt.
 */
int print_slice_timing() {
  static int instance_number = 0;
  FILE *time_file;
  time_file = fopen("times.txt", "a");
  fprintf(time_file, "time_slice %d = %d us\n", instance_number, 
    (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
  instance_number++;
  fclose(time_file);
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}
/*
 * Write dvfs timing information to times.txt.
 */
int print_dvfs_timing() {
  static int instance_number = 0;
  FILE *time_file;
  time_file = fopen("times.txt", "a");
  fprintf(time_file, "time_dvfs %d = %d us\n", instance_number, 
    (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
  instance_number++;
  fclose(time_file);
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}
/*
 * Write deadline information to times.txt.
 */
void print_deadline(int deadline_time){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "============ deadline time : %d us ===========\n", deadline_time);
    fclose(time_file);
}
void print_predicted_time(float predicted_exec_time){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "predicted time = %f\n", predicted_exec_time);
    fclose(time_file);
}
void print_exec_time(int exec_time){
    static int instance_number = 0;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "time %d = %d us\n", instance_number, exec_time);
    instance_number++;
    fclose(time_file);
}
void print_total_time(int exec_time){
    static int instance_number = 0;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "time_total %d = %d us\n", instance_number, exec_time);
    instance_number++;
    fclose(time_file);
}
void print_delay_time(int pre_delay_time, int delay_time){
    static int instance_number = 0;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "delay should be %d = %d us\n", instance_number, pre_delay_time);
    fprintf(time_file, "actual dealy is %d = %d us\n", instance_number, delay_time);
    instance_number++;
    fclose(time_file);
}
void print_current_core(int current_core){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "current_core : %d\n", current_core);
    fclose(time_file);
}

/*
 * Write start temperature
 */
void print_start_temperature() {
    static int instance_number = 0;
    FILE *fp_tmu; //File pointer of TMU file
    FILE *time_file;
    int c;
    time_file = fopen("times.txt", "a");
    if(NULL == (fp_tmu = fopen("/sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp", "r"))){
        fprintf(time_file, "TMU Read Error\n");
        return -1;
    }
    fprintf(time_file, "start_temperature %d\n", instance_number);
    fclose(time_file);
    print_file(fp_tmu);
    instance_number++;
}
/*
 * Write end temperature
 */
void print_end_temperature() {
    static int instance_number = 0;
    FILE *fp_tmu; //File pointer of TMU file
    FILE *time_file;
    int c;
    time_file = fopen("times.txt", "a");
    if(NULL == (fp_tmu = fopen("/sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp", "r"))){
        fprintf(time_file, "TMU Read Error\n");
        return -1;
    }
    fprintf(time_file, "end_temperature %d\n", instance_number);
    fclose(time_file);
    print_file(fp_tmu);
    instance_number++;
}
/*
 * Write file
 */
void print_file(FILE *file){
    int c;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    if(file){
        while((c = getc(file)) != EOF){
            fputs(c, time_file);
        }
        fclose(time_file);
        fclose(file);
    }
    return;
}
#endif

#endif
