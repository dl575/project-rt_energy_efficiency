/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"
#include "timing.h"

//---------------------modified by TJSong----------------------//
struct timeval start, end, moment;

#define MILLION 1000000L
//manually set below
#define CORE 1 //0:LITTLE, 1:big

#define PREDICT_EN 0 //0:prediction off, 1:prediction on
#define DELAY_EN 1 //0:delay off, 1:delay on
#define OVERHEAD_EN 0 //1:measure dvfs, slice timing

#define OVERHEAD_TIME 39126 //overhead deadline
#if OVERHEAD_EN
#define DEADLINE_TIME 51467 + OVERHEAD_TIME //deadline with overhead
#else
#define DEADLINE_TIME 51467 //deadline w/o overhead
#endif
#define GET_PREDICT 0 //to get prediction equation

#define DEBUG_EN 1 //debug information print on/off
//automatically set
#define MAX_FREQ ((CORE)?(2000000):(1400000))

FILE *fp_power; //File pointer of power of A7 (LITTLE) core or A15 (big) core power sensor file
FILE *fp_freq; //File pointer of freq of A7 (LITTLE) core or A15 (big) core power sensor file
float watt; //Value (Watt) at start point.
int khz; //Value (khz) at start point.

FILE *fp_max_freq; //File pointer scaling_max_freq
int predicted_freq = MAX_FREQ;

int slice_time;
int dvfs_time;

void fopen_all(void){
    if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq", "w"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
    }
    return;
}

void fclose_all(void){
   fclose(fp_max_freq);
    return;
}

void print_freq(void){
    if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return;
    }
    fscanf(fp_freq, "%d", &khz);
    printf("big core freq : %dkhz\n", khz);  
    fclose(fp_freq); 
    return;
}

inline void set_freq(float exec_time){
    //calculate predicted freq and round up by adding 99999
#if OVERHEAD_EN
    predicted_freq = exec_time * MAX_FREQ / (DEADLINE_TIME - OVERHEAD_TIME) + 99999;
#else
    predicted_freq = exec_time * MAX_FREQ / DEADLINE_TIME + 99999;
#endif
    //if less then 200000, just set it minimum (200000)
    predicted_freq = (predicted_freq < 200000)?(200000):(predicted_freq);
    //printf("predicted freq %d in set_freq function (rounded up)\n", predicted_freq); 
    //set maximum frequency, because performance governor always use maximum freq.
    fprintf(fp_max_freq, "%d", predicted_freq);
    //start_timing();
    fflush(fp_max_freq);
    //end_timing();
   // print_set_dvfs_timing();
    

    return;
}

//---------------------modified by TJSong----------------------//

void slice(int argc, char **argv)
{
//---------------------modified by TJSong----------------------//
#if GET_PREDICT
    start_timing();
#endif
//---------------------modified by TJSong----------------------//
  int loop_counter[47] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  FILE *fin;
  SHA_INFO sha_info;
{}
  if (argc < 2)
  {
    loop_counter[0]++;
    fin = stdin;
    {
      int i_rename0;
      BYTE data_rename0[8192];
      {
{}
{}
{}
{}
{}
        sha_info.count_lo = 0L;
{}
      }
      while ((i_rename0 = fread(data_rename0, 1, 8192, fin)) > 0)
      {
        loop_counter[1]++;
        {
          int count_rename5 = i_rename0;
          if ((sha_info.count_lo + (((LONG) count_rename5) << 3)) < sha_info.count_lo)
          {
            loop_counter[2]++;
{}
          }

          sha_info.count_lo += ((LONG) count_rename5) << 3;
{}
          while (count_rename5 >= SHA_BLOCKSIZE)
          {
            loop_counter[3]++;
{}
            {
              int i_rename10;
{}
{}
{}
{}
{}
{}
{}
              for (i_rename10 = 0; i_rename10 < 16; ++i_rename10)
              {
                loop_counter[4]++;
{}
              }

              for (i_rename10 = 16; i_rename10 < 80; ++i_rename10)
              {
                loop_counter[5]++;
{}
              }

{}
{}
{}
{}
{}
              for (i_rename10 = 0; i_rename10 < 20; ++i_rename10)
              {
                loop_counter[6]++;
{}
{}
{}
{}
{}
{}
              }

              for (i_rename10 = 20; i_rename10 < 40; ++i_rename10)
              {
                loop_counter[7]++;
{}
{}
{}
{}
{}
{}
              }

              for (i_rename10 = 40; i_rename10 < 60; ++i_rename10)
              {
                loop_counter[8]++;
{}
{}
{}
{}
{}
{}
              }

              for (i_rename10 = 60; i_rename10 < 80; ++i_rename10)
              {
                loop_counter[9]++;
{}
{}
{}
{}
{}
{}
              }

{}
{}
{}
{}
{}
            }
            //data_rename0 += SHA_BLOCKSIZE;
            count_rename5 -= SHA_BLOCKSIZE;
          }

{}
        }
      }

      {
        int count_rename6;
        LONG lo_bit_count_rename6;
{}
        lo_bit_count_rename6 = sha_info.count_lo;
{}
        count_rename6 = (int) ((lo_bit_count_rename6 >> 3) & 0x3f);
        ((BYTE *) sha_info.data)[count_rename6++] = 0x80;
        if (count_rename6 > 56)
        {
          loop_counter[10]++;
          memset(((BYTE *) (&sha_info.data)) + count_rename6, 0, 64 - count_rename6);
          {
            int i_rename11;
{}
{}
{}
{}
{}
{}
{}
            for (i_rename11 = 0; i_rename11 < 16; ++i_rename11)
            {
              loop_counter[11]++;
{}
            }

            for (i_rename11 = 16; i_rename11 < 80; ++i_rename11)
            {
              loop_counter[12]++;
{}
            }

{}
{}
{}
{}
{}
            for (i_rename11 = 0; i_rename11 < 20; ++i_rename11)
            {
              loop_counter[13]++;
{}
{}
{}
{}
{}
{}
            }

            for (i_rename11 = 20; i_rename11 < 40; ++i_rename11)
            {
              loop_counter[14]++;
{}
{}
{}
{}
{}
{}
            }

            for (i_rename11 = 40; i_rename11 < 60; ++i_rename11)
            {
              loop_counter[15]++;
{}
{}
{}
{}
{}
{}
            }

            for (i_rename11 = 60; i_rename11 < 80; ++i_rename11)
            {
              loop_counter[16]++;
{}
{}
{}
{}
{}
{}
            }

{}
{}
{}
{}
{}
          }
          memset(&sha_info.data, 0, 56);
        }
        else
        {
          memset(((BYTE *) (&sha_info.data)) + count_rename6, 0, 56 - count_rename6);
        }

{}
{}
        {
          int i_rename12;
{}
{}
{}
{}
{}
{}
{}
          for (i_rename12 = 0; i_rename12 < 16; ++i_rename12)
          {
            loop_counter[17]++;
{}
          }

          for (i_rename12 = 16; i_rename12 < 80; ++i_rename12)
          {
            loop_counter[18]++;
{}
          }

{}
{}
{}
{}
{}
          for (i_rename12 = 0; i_rename12 < 20; ++i_rename12)
          {
            loop_counter[19]++;
{}
{}
{}
{}
{}
{}
          }

          for (i_rename12 = 20; i_rename12 < 40; ++i_rename12)
          {
            loop_counter[20]++;
{}
{}
{}
{}
{}
{}
          }

          for (i_rename12 = 40; i_rename12 < 60; ++i_rename12)
          {
            loop_counter[21]++;
{}
{}
{}
{}
{}
{}
          }

          for (i_rename12 = 60; i_rename12 < 80; ++i_rename12)
          {
            loop_counter[22]++;
{}
{}
{}
{}
{}
{}
          }

{}
{}
{}
{}
{}
        }
      }
    }
    {
{}
    }
  }
  else
  {
    while (--argc)
    {
      loop_counter[23]++;
      fin = fopen(*(++argv), "rb");
      if (fin == NULL)
      {
        loop_counter[24]++;
        printf("error opening %s for reading\n", *argv);
      }
      else
      {
        {
          int i_rename2;
          BYTE data_rename2[8192];
          {
{}
{}
{}
{}
{}
            sha_info.count_lo = 0L;
{}
          }
          while ((i_rename2 = fread(data_rename2, 1, 8192, fin)) > 0)
          {
            loop_counter[25]++;
            {
              int count_rename8 = i_rename2;
              if ((sha_info.count_lo + (((LONG) count_rename8) << 3)) < sha_info.count_lo)
              {
                loop_counter[26]++;
{}
              }

              sha_info.count_lo += ((LONG) count_rename8) << 3;
{}
              while (count_rename8 >= SHA_BLOCKSIZE)
              {
                loop_counter[27]++;
{}
                {
                  int i_rename13;
{}
{}
{}
{}
{}
{}
{}
                  for (i_rename13 = 0; i_rename13 < 16; ++i_rename13)
                  {
                    loop_counter[28]++;
{}
                  }

                  for (i_rename13 = 16; i_rename13 < 80; ++i_rename13)
                  {
                    loop_counter[29]++;
{}
                  }

{}
{}
{}
{}
{}
                  for (i_rename13 = 0; i_rename13 < 20; ++i_rename13)
                  {
                    loop_counter[30]++;
{}
{}
{}
{}
{}
{}
                  }

                  for (i_rename13 = 20; i_rename13 < 40; ++i_rename13)
                  {
                    loop_counter[31]++;
{}
{}
{}
{}
{}
{}
                  }

                  for (i_rename13 = 40; i_rename13 < 60; ++i_rename13)
                  {
                    loop_counter[32]++;
{}
{}
{}
{}
{}
{}
                  }

                  for (i_rename13 = 60; i_rename13 < 80; ++i_rename13)
                  {
                    loop_counter[33]++;
{}
{}
{}
{}
{}
{}
                  }

{}
{}
{}
{}
{}
                }
                //data_rename2 += SHA_BLOCKSIZE;
                count_rename8 -= SHA_BLOCKSIZE;
              }

{}
            }
          }

          {
            int count_rename9;
            LONG lo_bit_count_rename9;
{}
            lo_bit_count_rename9 = sha_info.count_lo;
{}
            count_rename9 = (int) ((lo_bit_count_rename9 >> 3) & 0x3f);
            ((BYTE *) sha_info.data)[count_rename9++] = 0x80;
            if (count_rename9 > 56)
            {
              loop_counter[34]++;
              memset(((BYTE *) (&sha_info.data)) + count_rename9, 0, 64 - count_rename9);
              {
                int i_rename14;
{}
{}
{}
{}
{}
{}
{}
                for (i_rename14 = 0; i_rename14 < 16; ++i_rename14)
                {
                  loop_counter[35]++;
{}
                }

                for (i_rename14 = 16; i_rename14 < 80; ++i_rename14)
                {
                  loop_counter[36]++;
{}
                }

{}
{}
{}
{}
{}
                for (i_rename14 = 0; i_rename14 < 20; ++i_rename14)
                {
                  loop_counter[37]++;
{}
{}
{}
{}
{}
{}
                }

                for (i_rename14 = 20; i_rename14 < 40; ++i_rename14)
                {
                  loop_counter[38]++;
{}
{}
{}
{}
{}
{}
                }

                for (i_rename14 = 40; i_rename14 < 60; ++i_rename14)
                {
                  loop_counter[39]++;
{}
{}
{}
{}
{}
{}
                }

                for (i_rename14 = 60; i_rename14 < 80; ++i_rename14)
                {
                  loop_counter[40]++;
{}
{}
{}
{}
{}
{}
                }

{}
{}
{}
{}
{}
              }
              memset(&sha_info.data, 0, 56);
            }
            else
            {
              memset(((BYTE *) (&sha_info.data)) + count_rename9, 0, 56 - count_rename9);
            }

{}
{}
            {
              int i_rename15;
{}
{}
{}
{}
{}
{}
{}
              for (i_rename15 = 0; i_rename15 < 16; ++i_rename15)
              {
                loop_counter[41]++;
{}
              }

              for (i_rename15 = 16; i_rename15 < 80; ++i_rename15)
              {
                loop_counter[42]++;
{}
              }

{}
{}
{}
{}
{}
              for (i_rename15 = 0; i_rename15 < 20; ++i_rename15)
              {
                loop_counter[43]++;
{}
{}
{}
{}
{}
{}
              }

              for (i_rename15 = 20; i_rename15 < 40; ++i_rename15)
              {
                loop_counter[44]++;
{}
{}
{}
{}
{}
{}
              }

              for (i_rename15 = 40; i_rename15 < 60; ++i_rename15)
              {
                loop_counter[45]++;
{}
{}
{}
{}
{}
{}
              }

              for (i_rename15 = 60; i_rename15 < 80; ++i_rename15)
              {
                loop_counter[46]++;
{}
{}
{}
{}
{}
{}
              }

{}
{}
{}
{}
{}
            }
          }
        }
        {
{}
        }
{}
      }

    }

  }

{}
{}
  {
    //return_value = 0;
    goto print_loop_counter;
  }
  print_loop_counter:
  {
{}
    int i;
    printf("loop counter = (");
    for (i = 0; i < 47; i++)
      printf("%d, ", loop_counter[i]);
    printf(")\n");

{}
//float exec_time;
//exec_time = 2.058300*loop_counter[23] + 1988.040000*loop_counter[25] + -14.939600*loop_counter[27] + -504.579000*loop_counter[34] + 0.000000;
//printf("predicted time = %f\n", exec_time);

//---------------------modified by TJSong----------------------//
float exec_time;
exec_time = 73.413500*loop_counter[23] + 74.177900*loop_counter[25] + 0.108232*loop_counter[27] + -35.387000*loop_counter[34] + 0.000000;

#if GET_PREDICT
    end_timing();
    slice_time = print_slice_timing();
#endif

#if DEBUG_EN
    printf("predicted time = %f\n", exec_time);
#endif

#if GET_PREDICT
    start_timing();
#endif

    set_freq(exec_time); //do dvfs

#if GET_PREDICT
    end_timing();
    dvfs_time = print_dvfs_timing();
#endif
//---------------------modified by TJSong----------------------//
  }

}

int main(int argc, char **argv)
{
    FILE *fin;
    SHA_INFO sha_info;

    fopen_all();//TJSong 
    printf("============ deadline time : %d us ===========\n", DEADLINE_TIME);//TJSong

#if !PREDICT_EN
    moment_timing();
    printf("moment_start : %lu us\n", moment.tv_sec * MILLION + moment.tv_usec);
    start_timing();
#else //PREDICT_EN
    slice(argc, argv);
//    end_timing();//slice+dvfs
#if !OVERHEAD_EN
    moment_timing();
    printf("moment_start : %lu us\n", moment.tv_sec * MILLION + moment.tv_usec);
    start_timing();
#endif
//    int overhead_time = exec_timing();
#endif

#if GET_PREDICT
    slice(argc, argv);
    start_timing(); //overwrite start timing
#endif

#if DEBUG_EN
    print_freq(); //[DEBUG] check frequency 
#endif

    if (argc < 2) {
	fin = stdin;
	sha_stream(&sha_info, fin);
	sha_print(&sha_info);
    } else {
	while (--argc) {
	    fin = fopen(*(++argv), "rb");
	    if (fin == NULL) {
		printf("error opening %s for reading\n", *argv);
	    } else {
		sha_stream(&sha_info, fin);
		sha_print(&sha_info);
		fclose(fin);
	    }
	}
    }

    end_timing();
//---------------------modified by TJSong----------------------//
    int exec_time = exec_timing();
    int delay_time = 0;
    static int instance_number = 0;
#if DELAY_EN
    printf("exec_time_before_delay %d = %d us\n", instance_number, exec_time); //[DEBUG]
//    #if OVERHEAD_EN
//    if( (delay_time = DEADLINE_TIME - exec_time - slice_time - dvfs_time) > 0 ){
//    #else //OEVERHEAD_EN
    if( (delay_time = DEADLINE_TIME - exec_time) > 0 ){
//    #endif //OVERHEAD_EN
        printf("calculated delay is %d us\n", delay_time); //[DEBUG]
        start_timing();  
        usleep(delay_time);
        end_timing();
        delay_time = exec_timing();
        printf("actually delayed by %d us\n", delay_time); //[DEBUG]
        printf("total_time %d = %d us\n", instance_number, exec_time + delay_time);
    }else
        printf("total_time %d = %d us\n", instance_number, exec_time);
#else //DELAY_EN
    printf("total_time %d = %d us\n", instance_number, exec_time);
#endif //DELAY_EN
    moment_timing();
    printf("moment_end : %lu us\n", moment.tv_sec * MILLION + moment.tv_usec);
//    print_power();//TJSong
    instance_number++;
    fclose_all();//TJSong
//---------------------modified by TJSong----------------------//
   return(0);
}
