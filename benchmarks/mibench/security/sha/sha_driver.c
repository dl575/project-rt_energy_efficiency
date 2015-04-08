/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"
#include "timing.h"
#include "my_common.h"

//---------------------modified by TJSong----------------------//
//set global variables
struct timeval start, end, moment;
int slice_time = 0;
int dvfs_time = 0;

//define benchmarks-depenent varaibles & constants
#if CORE //big
#define OVERHEAD_TIME 47008 //overhead deadline
#define AVG_OVERHEAD_TIME 25529 //avg overhead deadline
#define DEADLINE_TIME 51035 + OVERHEAD_TIME //max_exec + max_overhead
#define MAX_DVFS_TIME 2877 //max dvfs time
#define AVG_DVFS_TIME 427 //average dvfs time
#else //LITTLE
#define OVERHEAD_TIME 76895 //overhead deadline
#define AVG_OVERHEAD_TIME 24442 //avg overhead deadline
#define DEADLINE_TIME 123765 + OVERHEAD_TIME //max_exec + max_overhead
#define MAX_DVFS_TIME 3384 //max dvfs time
#define AVG_DVFS_TIME 1866 //average dvfs time
#endif
//---------------------modified by TJSong----------------------//

float slice(int argc, char **argv)
{
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
#if PREDICT_EN || DEBUG_EN
    int i;
    printf("loop counter = (");
    for (i = 0; i < 47; i++)
      printf("%d, ", loop_counter[i]);
    printf(")\n");
#endif
{}
  }
  {
    predict_exec_time:
    ;

    float exec_time;
#if CORE //big
exec_time = 2.058300*loop_counter[23] + 1988.040000*loop_counter[25] + -14.939600*loop_counter[27] + -504.579000*loop_counter[34] + 0.000000;
#else //LITTLE
exec_time = 2.058300*loop_counter[23] + 1988.040000*loop_counter[25] + -14.939600*loop_counter[27] + -504.579000*loop_counter[34] + 0.000000;
#endif
    return exec_time;
  }


}

int main(int argc, char **argv)
{
    FILE *fin;
    SHA_INFO sha_info;
//---------------------modified by TJSong----------------------//
    fopen_all(); //fopen for frequnecy file
    print_deadline(DEADLINE_TIME); //print deadline 
//---------------------modified by TJSong----------------------//

//---------------------modified by TJSong----------------------//
    // Perform slicing and prediction
    float predicted_exec_time = 0.0;
    /*
        CASE 0 = to get prediction equation
        CASE 1 = to get execution deadline
        CASE 2 = to get overhead deadline
        CASE 3 = running on default linux governors
        CASE 4 = running on our prediction with overhead 
        CASE 5 = running on our prediction without overhead 
    */
    #if GET_PREDICT /* CASE 0 */
        predicted_exec_time = slice(argc, argv); //slice
    #endif
    #if GET_DEADLINE /* CASE 1 */
        //nothing
    #endif
    #if GET_OVERHEAD /* CASE 2 */
        start_timing();
        predicted_exec_time = slice(argc, argv); //slice
        end_timing();
        slice_time = print_slice_timing();

        start_timing();
        set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
        end_timing();
        dvfs_time = print_dvfs_timing();
    #endif
    #if !PREDICT_EN /* CASE 3 */
        //slice_time=0; dvfs_time=0;
        moment_timing_print(0); //moment_start
    #endif
    #if PREDICT_EN && OVERHEAD_EN /* CASE 4 */
        moment_timing_print(0); //moment_start
        
        start_timing();
        predicted_exec_time = slice(argc, argv); //slice
        end_timing();
        slice_time = print_slice_timing();

        start_timing();
        set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
        end_timing();
        dvfs_time = print_dvfs_timing();
    #endif
    #if PREDICT_EN && !OVERHEAD_EN /* CASE 5 */
        start_timing();
        predicted_exec_time = slice(argc, argv); //slice
        end_timing();
        slice_time = print_slice_timing();
        
        moment_timing_print(0); //moment_start

        start_timing();
        set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
        end_timing();
        dvfs_time = print_dvfs_timing();
    #endif
    // Write out predicted time & print out frequency used
    #if DEBUG_EN
        print_predicted_time(predicted_exec_time);
        print_freq(); 
    #endif
//---------------------modified by TJSong----------------------//

    start_timing();
    
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

    #if GET_PREDICT /* CASE 0 */
        print_exec_time(exec_time);
    #endif
    #if GET_DEADLINE /* CASE 1 */
        print_exec_time(exec_time);
    #endif
    #if GET_OVERHEAD /* CASE 2 */
        //nothing
    #endif
    #if !PREDICT_EN || (PREDICT_EN && OVERHEAD_EN) || (PREDICT_EN && !OVERHEAD_EN) /* CASE 3, 4, and 5 */
        if(DELAY_EN && ((delay_time = DEADLINE_TIME - exec_time - slice_time - dvfs_time) > 0)){
            start_timing();
            usleep(delay_time);
            end_timing();
            delay_time = exec_timing();
        }else
            delay_time = 0;
        print_total_time(exec_time + slice_time + dvfs_time + delay_time);
        moment_timing_print(1); //moment_end
    #endif
    fclose_all();//TJSong
//---------------------modified by TJSong----------------------//
   return(0);
}
