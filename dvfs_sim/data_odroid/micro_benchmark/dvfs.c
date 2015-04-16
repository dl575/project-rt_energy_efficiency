#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define CORE 1 //0:LITTLE, 1:big
//automatically set
#define DVFS_EN 1 //1:change dvfs, 0:don't change dvfs (e.g., not running on ODROID)
#define MAX_FREQ ((CORE)?(2000000):(1400000))
#define MAX_CNT 100

FILE *fp_max_freq; //File pointer scaling_max_freq


struct timeval start, end;
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
 * Print timing information to stdout.
 */
int exec_timing() {
//  static int instance_number = 0;
//  printf("time %d = %d us\n", instance_number, 
  return (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec);
}


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
    printf("%d\t", khz/1000);  
#else //LITTLE
    if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
    }
    fscanf(fp_freq, "%d", &khz);
    printf("%d\t", khz/1000);  
#endif
    fclose(fp_freq);
    return;
}



void set_freq(int to, float predicted_exec_time, int slice_time, int deadline_time, int avg_dvfs_time){
//void set_freq(int to){
    int predicted_freq = MAX_FREQ;
    //calculate predicted freq and round up by adding 99999
    predicted_freq = predicted_exec_time * MAX_FREQ / (deadline_time - slice_time - avg_dvfs_time) + 99999;
    //if less then 200000, just set it minimum (200000)
    predicted_freq = (predicted_freq < 200000 || predicted_exec_time <= 0)?(200000):(predicted_freq);
   // printf("############### %d #########3\n", predicted_freq);

    fprintf(fp_max_freq, "%d", to);
    fflush(fp_max_freq);
    return;
}

int main(){
    int from, to, dvfs_time;
    float a;
    int b;
    time_t t;
    int i, j, cnt;
#if CORE
    int arr[19][19]={0};
    int avg_arr[19][19]={0};
#else
    int arr[13][13]={0};
    int avg_arr[13][13]={0};
#endif

    fopen_all();
    srand((unsigned)time(&t));
for(cnt=0; cnt<MAX_CNT; cnt++){
    i=0;
#if CORE
    for (from = 200; from <= 2000; from+=100)
#else
    for (from = 200; from <= 1400; from+=100)
#endif
    {
        j=0;
#if CORE
        for(to=200; to <=2000; to+=100){
#else
        for(to=200; to <=1400; to+=100){
#endif
            a = (float)(rand()%10000+1);
            b = rand()%10000+1;
            set_freq(1000*from, a, b, 20000, 2000);
            usleep(1000);

            print_freq();

            start_timing();
            set_freq(1000*to, a, b, 20000, 2000);
            end_timing();
            arr[i][j] += exec_timing();
            avg_arr[i][j] = arr[i][j]/(cnt+1);
            
            print_freq();
            printf("%d\n", avg_arr[i][j]);
            j++;
        }
        i++;
    }

}
    fclose_all();
    return 0;
}
