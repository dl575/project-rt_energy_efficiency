#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __TIMING_H__
#define __TIMING_H__
#include "deadline_big.h"
#include "deadline_little.h"
#include "my_common.h"

extern struct timeval start, end, moment;

void start_timing();
void end_timing();
void moment_timing();
void print_timing();
int print_slice_timing();
int print_dvfs_timing();
int fprint_slice_timing();
int fprint_dvfs_timing();
int exec_timing();
void write_timing();
void write_array(int *array, int array_len);
void init_time_file();

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
 * Moment timing, fprint moment.
 */
void moment_timing_fprint(int start_end) {
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
int print_slice_timing() {
  static int instance_number = 0;
  printf("time_slice %d = %d us\n", instance_number, 
    (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
  instance_number++;
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}

/*
 * Print timing information to stdout.
 */
int print_dvfs_timing() {
  static int instance_number = 0;
  printf("time_dvfs %d = %d us\n", instance_number, 
    (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));
  instance_number++;
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}

/*
 * Write timing information to times.txt.
 */
int fprint_slice_timing() {
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
 * Write timing information to times.txt.
 */
int fprint_dvfs_timing() {
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
 * Print timing information to stdout.
 */
int exec_timing() {
//  static int instance_number = 0;
//  printf("time %d = %d us\n", instance_number, 
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}


/*
 * Write timing information out to a file.
 */
void write_timing() {
  static int instance_number = 0;

  // Open file for writing
  FILE *time_file;
  time_file = fopen("times.txt", "a");
  if (time_file == NULL) {
    printf("Error opening times.txt!\n");
    exit(1);
  }

  // Write time
  int time = (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
  fprintf(time_file, "time %d = %d us\n", instance_number, time);
  instance_number++;

  fclose(time_file);
}

/*
 * Write the passed string to times.txt.
 */
void write_string(char *string) {
  FILE *time_file;
  time_file = fopen("times.txt", "a");
  if (time_file == NULL) {
    printf("Error opening times.txt!\n");
    exit(1);
  }
  fprintf(time_file, "%s", string);
  fclose(time_file);
}

/*
 * Write the passed integer array to the timing file.
 */
void write_array(int *array, int array_len) {
  static int instance_number = 0;

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
  instance_number++;

  fclose(time_file);
}

void init_time_file() {
  FILE *time_file;
  time_file = fopen("times.txt", "w");
  fclose(time_file);
}

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
    static int instance_number = 1;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "time %d = %d us\n", instance_number, exec_time);
    instance_number++;
    fclose(time_file);
}

void print_exec_time(int exec_time){
    static int instance_number = 1;
    printf("time %d = %d us\n", instance_number, exec_time);
    instance_number++;
}

void fprint_total_time(int exec_time){
    static int instance_number = 1;
    FILE *time_file;
    time_file = fopen("times.txt", "a");
    fprintf(time_file, "time_total %d = %d us\n", instance_number, exec_time);
    instance_number++;
    fclose(time_file);
}

void print_total_time(int exec_time){
    static int instance_number = 1;
    printf("time_total %d = %d us\n", instance_number, exec_time);
    instance_number++;
}


#endif
