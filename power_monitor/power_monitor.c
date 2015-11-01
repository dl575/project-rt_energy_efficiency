#include <stdio.h>  /* for printf */
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <sys/time.h>
#include <time.h>   /* for clock_gettime */
#include <limits.h>   /* for INT_MAX */

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


//manually set below
#define CORE 0 //0:LITTLE, 1:big

#define BILLION 1000000000L
#define MILLION 1000000L
#define THOUSAND 1000L

#define UPDATED_PERIOD (int)(263808*0.97) // 4705us is updated_period of sensors

int main(void)
{
    FILE *fp_power; //File pointer of power of A15 (big) core power sensor file
    float watt;
    
    struct timeval s;
    
#if CORE //big
    if(NULL == (fp_power = fopen("/sys/bus/i2c/drivers/INA231/2-0040/sensor_W", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return -1;
    }
    fclose(fp_power); 
#else //LITTLE    
    if(NULL == (fp_power = fopen("/sys/bus/i2c/drivers/INA231/2-0045/sensor_W", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return -1;
    }
    fclose(fp_power); 
#endif

    while(1){
        gettimeofday(&s, NULL);

#if CORE //big
        fp_power = fopen("/sys/bus/i2c/drivers/INA231/2-0040/sensor_W", "r");
        if(fscanf(fp_power, "%f", &watt) < 0)
            return -1;
        fclose(fp_power);
        printf("moment : %llu us, big core power : %fW\n", (unsigned long long int)s.tv_sec * MILLION + (unsigned long long int)s.tv_usec, watt);
#else //LITTLE    
        fp_power = fopen("/sys/bus/i2c/drivers/INA231/2-0045/sensor_W", "r");
        if(fscanf(fp_power, "%f", &watt) < 0)
            return -1;
        fclose(fp_power);
        printf("moment : %llu us, little core power : %fW\n", (unsigned long long int)s.tv_sec * MILLION + (unsigned long long int)s.tv_usec, watt);
#endif

        my_usleep(UPDATED_PERIOD); //wait updated_frequency
    }

    return 0;
}
