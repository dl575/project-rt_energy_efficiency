#include <stdio.h>  /* for printf */
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */

//manually set below
#define CORE 1 //0:LITTLE, 1:big

#define BILLION 1000000000L
#define MILLION 1000000L
#define THOUSAND 1000L

#define UPDATED_PERIOD (int)(4705*0.97) // 4705us is updated_period of sensors

int main(void)
{
    FILE *fp_power; //File pointer of power of A15 (big) core power sensor file
    float watt;
    
    struct timeval now;
    struct timeval s;
    int rc;

#if CORE //big
    if(NULL == (fp_power = fopen("/sys/bus/i2c/drivers/INA231/3-0040/sensor_W", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return -1;
    }
    fclose(fp_power); 
#else //LITTLE    
    if(NULL == (fp_power = fopen("/sys/bus/i2c/drivers/INA231/3-0045/sensor_W", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return -1;
    }
    fclose(fp_power); 
#endif

    now.tv_sec=0;
    now.tv_usec=0;
    rc = settimeofday(&now, NULL);
    if (rc==0){
        printf("settimeofday success\n");
    }else{  
        printf("settimeofday failed\n");
        return -1;
    }

    while(1){
        gettimeofday(&s, NULL);

#if CORE //big
        fp_power = fopen("/sys/bus/i2c/drivers/INA231/3-0040/sensor_W", "r");
        fscanf(fp_power, "%f", &watt);
        fclose(fp_power);
        printf("moment : %lu us, big core power : %fW\n", s.tv_sec * MILLION + s.tv_usec, watt);
#else //LITTLE    
        fp_power = fopen("/sys/bus/i2c/drivers/INA231/3-0045/sensor_W", "r");
        fscanf(fp_power, "%f", &watt);
        fclose(fp_power);
        printf("moment : %lu us, LITTLE core power : %fW\n", s.tv_sec * MILLION + s.tv_usec, watt);
#endif

        usleep(UPDATED_PERIOD); //4705us is updated_frequency
    }

    return 0;
}
