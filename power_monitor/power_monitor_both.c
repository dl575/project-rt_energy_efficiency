#include <stdio.h>  /* for printf */
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */
#include <limits.h>   /* for INT_MAX */


#define BILLION 1000000000L
#define MILLION 1000000L
#define THOUSAND 1000L

#define UPDATED_PERIOD (int)(4705*0.97) // 4705us is updated_period of sensors

int main(void)
{
    FILE *fp_power_big; //File pointer of power of A15 (big) core power sensor file
    FILE *fp_power_little; //File pointer of power of A15 (big) core power sensor file
    float watt_big;
    float watt_little;
    
    struct timeval now;
    struct timeval s;
    int rc;
    
    //big
    if(NULL == (fp_power_big = fopen("/sys/bus/i2c/drivers/INA231/2-0040/sensor_W", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return -1;
    }
    fclose(fp_power_big); 
    //LITTLE    
    if(NULL == (fp_power_little = fopen("/sys/bus/i2c/drivers/INA231/2-0045/sensor_W", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return -1;
    }
    fclose(fp_power_little); 
    while(1){

        //big
        fp_power_big = fopen("/sys/bus/i2c/drivers/INA231/2-0040/sensor_W", "r");
        //LITTLE    
        fp_power_little = fopen("/sys/bus/i2c/drivers/INA231/2-0045/sensor_W", "r");
        
        gettimeofday(&s, NULL);
        fscanf(fp_power_big, "%f", &watt_big);
        fscanf(fp_power_little, "%f", &watt_little);
        
        printf("big moment : %llu us, big core power : %fW\n", (unsigned long long int)s.tv_sec * MILLION + (unsigned long long int)s.tv_usec, watt_big);
        printf("little moment : %llu us, little core power : %fW\n", (unsigned long long int)s.tv_sec * MILLION + (unsigned long long int)s.tv_usec, watt_little);
        fclose(fp_power_big);
        fclose(fp_power_little);

        usleep(UPDATED_PERIOD); //4705us is updated_frequency
    }

    return 0;
}
