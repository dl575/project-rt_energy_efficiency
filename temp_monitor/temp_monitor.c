#include <stdio.h>  /* for printf */
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */
#include <limits.h>   /* for INT_MAX */

#define BILLION 1000000000L
#define MILLION 1000000L
#define THOUSAND 1000L

#define UPDATED_PERIOD (int)(4705*0.97) // 4705us is updated_period of sensors
void print_file(FILE *file);

int main(void)
{
    FILE *fp_tmu; //File pointer of TMU file
    char *celcius;
    
    struct timeval s;
/*
# Note Configuration for CPU Core Temperature
# This file is written on the following format:
# CPU4 CPU5 CPU6 CPU7 GPU
TMU_FILE=`sudo cat /sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp`

# We need to slip those in nice variables...
CPU4_TEMP=$((`echo $TMU_FILE | awk '{printf $3}'`/1000))" C"
CPU5_TEMP=$((`echo $TMU_FILE | awk '{printf $6}'`/1000))" C"
CPU6_TEMP=$((`echo $TMU_FILE | awk '{printf $9}'`/1000))" C"
CPU7_TEMP=$((`echo $TMU_FILE | awk '{printf $12}'`/1000))" C"
GPU_TEMP=$((`echo $TMU_FILE | awk '{printf $15}'`/1000))" C"
*/
    
    if(NULL == (fp_tmu = fopen("/sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp", "r"))){
        printf("ERROR : TMU FILE READ FAILED\n");
        return -1;
    }
    fclose(fp_tmu); 
    
    while(1){
        gettimeofday(&s, NULL);

        fp_tmu = fopen("/sys/bus/platform/drivers/exynos-tmu/10060000.tmu/temp", "r");
        printf("moment : %llu us\n", (unsigned long long int)s.tv_sec * MILLION + (unsigned long long int)s.tv_usec);
        print_file(fp_tmu);
        
        usleep(UPDATED_PERIOD); //4705us is updated_frequency
    }

    return 0;
}

void print_file(FILE *file){
    int c;
    if(file){
        while((c = getc(file)) != EOF)
            putchar(c);
        fclose(file);
    }
    return;
}
