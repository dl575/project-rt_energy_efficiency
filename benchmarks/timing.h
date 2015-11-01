#ifndef __TIMING_H__
#define __TIMING_H__

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "deadline.h"
#include "my_common.h"

#define MILLION 1000000L

extern struct timeval start, end, moment;
extern struct timeval start_local, end_local;
extern int client_join;

//Define function names
//all benchmarks use below common timing functions
extern void start_timing();
extern void end_timing();
extern int exec_timing();
extern void start_timing_local();
extern void end_timing_local();
extern int exec_timing_local();
extern void init_time_file();
extern void my_usleep(unsigned long us);

//These functions are defined differently by F_PRINT
extern void print_array(int *array, int array_len);
extern void print_timing();
extern void print_timing_local();
extern void moment_timing_print();
extern int print_slice_timing();
extern int print_dvfs_timing();
extern void print_deadline(int deadline_time);
extern void print_predicted_time(int predicted_exec_time);
extern void print_exec_time(int exec_time);
extern void print_total_time(int exec_time);
extern void print_delay_time(int delay_time, int actual_delay_time);
extern void print_update_time(int update_time);
extern void print_current_core(int current_core, int big_little_cnt);
extern void print_est_time(int T_est_big, int T_est_little);
extern void print_enter();
extern void print_freq_power(int f_new_big, int f_new_little, float power_big, float power_little);

extern void print_start_temperature();
extern void print_end_temperature();
extern void print_file(FILE *file);

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
 * Start of section to time.
 */
void start_timing_local() {
  gettimeofday(&start_local, NULL);
}
/*
 * Stop timing, record end time.
 */
void end_timing_local() {
  gettimeofday(&end_local, NULL);
}
/*
 * Return excute time
 */
int exec_timing_local() {
  return (int)(end_local.tv_sec - start_local.tv_sec)*1000000 +
	  (int)(end_local.tv_usec - start_local.tv_usec);
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
 * Print timing information to stdout.
 */
void print_timing_local() {
  static int instance_number = 0;
  printf("time_local %d = %d us\n", instance_number, 
    (int)(end_local.tv_sec - start_local.tv_sec)*1000000 +
	(int)(end_local.tv_usec - start_local.tv_usec));
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
void print_predicted_time(int predicted_exec_time){
    printf("predicted time = %d\n", predicted_exec_time);
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
void print_delay_time(int pre_delay_time, int actual_delay_time){
    static int instance_number = 0;
    printf("delay should be %d = %d us\n", instance_number, pre_delay_time);
    printf("actual dealy is %d = %d us\n", instance_number, actual_delay_time);
    instance_number++;
}
void print_update_time(int exec_time){
    static int instance_number = 0;
    printf("time_update %d = %d us\n", instance_number, exec_time);
    instance_number++;
}
void print_current_core(int current_core, int big_little_cnt){
    printf("current_core : %d\n", current_core);
    if(current_core == 1)
        printf("big %d times\n", big_little_cnt);
    else if(current_core ==0)
        printf("little %d times\n", big_little_cnt);
}
void print_est_time(int T_est_big, int T_est_little){
	if(T_est_little == -99)//if -99, it's not hetero
		printf("%d, ", T_est_big);//it's not big only
	else
		printf("%d:%d, ", T_est_big, T_est_little);
}
void print_enter(){
	printf("\n");
}
void print_freq_power(int f_new_big, int f_new_little, float power_big, float power_little){
    printf("\n");
	if(f_new_little == -99){//if -99, it's not hetero
		printf("f_new (Mhz) = %d\n", f_new_big);//it's not big only
	}else{
		printf("f_new big:little (Mhz) = %d:%d\n", f_new_big, f_new_little);
		printf("power big:little (W) = %f:%f\n", power_big, power_little);
	}
}

/*
 * Print start temperature
 */
void print_start_temperature() {
    static int instance_number = 0;
    FILE *fp_tmu; //File pointer of TMU file
    if(NULL == (fp_tmu = fopen("/sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp", "r"))){
        printf("TMU Read Error\n");
        return;
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
        return;
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
 * Print timing information to stdout.
 */
void print_timing_local() {
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    static int instance_number = 0;
    fprintf(time_file, "time_local %d = %d us\n", instance_number, 
        (int)(end_local.tv_sec - start_local.tv_sec)*1000000 +
		(int)(end_local.tv_usec - start_local.tv_usec));
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
void print_predicted_time(int predicted_exec_time){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "predicted time = %d\n", predicted_exec_time);
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
void print_delay_time(int pre_delay_time, int actual_delay_time){
    static int instance_number = 0;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "delay should be %d = %d us\n", instance_number, pre_delay_time);
    fprintf(time_file, "actual dealy is %d = %d us\n", instance_number,
			actual_delay_time);
    instance_number++;
    fclose(time_file);
}
void print_update_time(int exec_time){
    static int instance_number = 0;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "time_update %d = %d us\n", instance_number, exec_time);
    instance_number++;
    fclose(time_file);
}
void print_current_core(int current_core, int big_little_cnt){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "current_core : %d\n", current_core);
    if(current_core == 1)
        fprintf(time_file, "big %d times\n", big_little_cnt);
    else if(current_core ==0)
        fprintf(time_file, "little %d times\n", big_little_cnt);
    fclose(time_file);
}
void print_est_time(int T_est_big, int T_est_little){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
	if(T_est_little == -99)//if -99, it's not hetero
		fprintf(time_file, "%d, ", T_est_big);
	else
		fprintf(time_file, "%d:%d, ", T_est_big, T_est_little);
    fclose(time_file);
}
void print_enter(){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
	fprintf(time_file, "\n");
    fclose(time_file);
}
void print_freq_power(int f_new_big, int f_new_little, float power_big, float power_little){
    FILE *time_file;
    time_file = fopen("times.txt", "a");
	if(f_new_little == -99){//if -99, it's not hetero
		fprintf(time_file, "f_new (Mhz) = %d\n", f_new_big);//it's not big only
	}else{
		fprintf(time_file, "f_new big:little (Mhz) = %d:%d\n", f_new_big, f_new_little);
		fprintf(time_file, "power big:little (W) = %f:%f\n", power_big, power_little);
	}
    fclose(time_file);
}

/*
 * Write start temperature
 */
void print_start_temperature() {
    static int instance_number = 0;
    FILE *fp_tmu; //File pointer of TMU file
    FILE *time_file;
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
