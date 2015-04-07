#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __TIMING_H__
#define __TIMING_H__

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
 * Moment timing, record end time.
 */
void moment_timing() {
  gettimeofday(&moment, NULL);
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

#endif
