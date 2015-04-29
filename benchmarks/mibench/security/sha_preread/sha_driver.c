/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"
#include "timing.h"

float sha_stream_slice(SHA_INFO *sha_info, char *file_buffer, int flen)
{
  int loop_counter[23] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int i;
  {
    sha_info->count_lo = 0L;
    return0:
    ;

  }
  int j;
  for (j = 0; j < flen; j += 8192)
  {
    loop_counter[0]++;
    i = flen - j;
    if (i > 8192)
    {
      loop_counter[1]++;
      i = 8192;
    }

    {
      int count_rename1 = i;
      if ((sha_info->count_lo + (((LONG) count_rename1) << 3)) < sha_info->count_lo)
      {
        loop_counter[2]++;
      }

      sha_info->count_lo += ((LONG) count_rename1) << 3;
      while (count_rename1 >= 64)
      {
        loop_counter[3]++;
        {
          int i_rename3;
          for (i_rename3 = 0; i_rename3 < 16; ++i_rename3)
          {
            loop_counter[4]++;
          }

          for (i_rename3 = 16; i_rename3 < 80; ++i_rename3)
          {
            loop_counter[5]++;
          }

          for (i_rename3 = 0; i_rename3 < 20; ++i_rename3)
          {
            loop_counter[6]++;
          }

          for (i_rename3 = 20; i_rename3 < 40; ++i_rename3)
          {
            loop_counter[7]++;
          }

          for (i_rename3 = 40; i_rename3 < 60; ++i_rename3)
          {
            loop_counter[8]++;
          }

          for (i_rename3 = 60; i_rename3 < 80; ++i_rename3)
          {
            loop_counter[9]++;
          }

          return3:
          ;

        }
        count_rename1 -= 64;
      }

      return1:
      ;

    }
  }

  {
    int count_rename2;
    LONG lo_bit_count_rename2;
    lo_bit_count_rename2 = sha_info->count_lo;
    count_rename2 = (int) ((lo_bit_count_rename2 >> 3) & 0x3f);
    ((BYTE *) sha_info->data)[count_rename2++] = 0x80;
    if (count_rename2 > 56)
    {
      loop_counter[10]++;
      memset(((BYTE *) (&sha_info->data)) + count_rename2, 0, 64 - count_rename2);
      {
        int i_rename4;
        for (i_rename4 = 0; i_rename4 < 16; ++i_rename4)
        {
          loop_counter[11]++;
        }

        for (i_rename4 = 16; i_rename4 < 80; ++i_rename4)
        {
          loop_counter[12]++;
        }

        for (i_rename4 = 0; i_rename4 < 20; ++i_rename4)
        {
          loop_counter[13]++;
        }

        for (i_rename4 = 20; i_rename4 < 40; ++i_rename4)
        {
          loop_counter[14]++;
        }

        for (i_rename4 = 40; i_rename4 < 60; ++i_rename4)
        {
          loop_counter[15]++;
        }

        for (i_rename4 = 60; i_rename4 < 80; ++i_rename4)
        {
          loop_counter[16]++;
        }

        return4:
        ;

      }
      memset(&sha_info->data, 0, 56);
    }
    else
    {
      memset(((BYTE *) (&sha_info->data)) + count_rename2, 0, 56 - count_rename2);
    }

    {
      int i_rename5;
      for (i_rename5 = 0; i_rename5 < 16; ++i_rename5)
      {
        loop_counter[17]++;
      }

      for (i_rename5 = 16; i_rename5 < 80; ++i_rename5)
      {
        loop_counter[18]++;
      }

      for (i_rename5 = 0; i_rename5 < 20; ++i_rename5)
      {
        loop_counter[19]++;
      }

      for (i_rename5 = 20; i_rename5 < 40; ++i_rename5)
      {
        loop_counter[20]++;
      }

      for (i_rename5 = 40; i_rename5 < 60; ++i_rename5)
      {
        loop_counter[21]++;
      }

      for (i_rename5 = 60; i_rename5 < 80; ++i_rename5)
      {
        loop_counter[22]++;
      }

      return5:
      ;

    }
    return2:
    ;

  }
  {
    print_loop_counter:
#if GET_PREDICT || DEBUG_EN
    ;

    {
      printf("loop counter = (");
      int i;
      for (i = 0; i < 23; i++)
        printf("%d, ", loop_counter[i]);

      printf(")\n");
    }
    print_loop_counter_end:
#endif
    ;

  }
  {
    predict_exec_time:
    ;
    float exec_time;
#if CORE
    exec_time = -24.262800*loop_counter[1] + 0.801681*loop_counter[3] + -21.147600*loop_counter[10] + 6.591040*loop_counter[17] + 0.000000;
#else
    exec_time = -24.262800*loop_counter[1] + 0.801681*loop_counter[3] + -21.147600*loop_counter[10] + 6.591040*loop_counter[17] + 0.000000;
#endif 
   return exec_time;
  }
}

int main(int argc, char **argv)
{
    FILE *fin;
    SHA_INFO sha_info;

//---------------------modified by TJSong----------------------//
    int exec_time = 0;
    if(check_define()==ERROR_DEFINE){
        printf("%s", "DEFINE ERROR!!\n");
        return ERROR_DEFINE;
    }
//---------------------modified by TJSong----------------------//

    if (argc < 2) {
      printf("stdin read not currently supported for slicing. Please implement.\n");
      exit(1);

      /*
      fin = stdin;
      sha_stream(&sha_info, fin);
      sha_print(&sha_info);
      */
    } else {
      while (--argc) {
        fin = fopen(*(++argv), "rb");
        if (fin == NULL) {
          printf("error opening %s for reading\n", *argv);
            exit(1);
        } else {
          // Get file length
          int flen;
          fseek(fin, 0, SEEK_END);
          fgetpos(fin, &flen);
          fseek(fin, 0, SEEK_SET);
          // Allocate buffer
          char *file_buffer = malloc(sizeof(char) * (flen + 1));
          // Read file into buffer
          size_t newLen = fread(file_buffer, sizeof(char), flen, fin);
          if (newLen == 0) {
            printf("Error reading file\n");
            exit(1);
          } else {
            file_buffer[++newLen] = '\0';
          }
          fclose(fin);

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
            CASE 4 = running on our prediction
            CASE 5 = running on oracle
            CASE 6 = running on pid
        */
        #if GET_PREDICT /* CASE 0 */
            predicted_exec_time = sha_stream_slice(&sha_info, file_buffer, flen);
        #elif GET_DEADLINE /* CASE 1 */
            //nothing
        #elif GET_OVERHEAD /* CASE 2 */
            start_timing();
            predicted_exec_time = sha_stream_slice(&sha_info, file_buffer, flen);
            end_timing();
            slice_time = print_slice_timing();

            start_timing();
            set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            end_timing();
            dvfs_time = print_dvfs_timing();
        #elif !ORACLE_EN && !PID_EN && !PREDICT_EN /* CASE 3 */
            //slice_time=0; dvfs_time=0;
            moment_timing_print(0); //moment_start
        #elif !ORACLE_EN && !PID_EN && PREDICT_EN /* CASE 4 */
            moment_timing_print(0); //moment_start
            
            start_timing();
            predicted_exec_time = sha_stream_slice(&sha_info, file_buffer, flen);
            end_timing();
            slice_time = print_slice_timing();
            
            start_timing();
            set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            end_timing();
            dvfs_time = print_dvfs_timing();

            moment_timing_print(1); //moment_start
        #elif ORACLE_EN /* CASE 5 */
            //slice_time=0;
            static int job_cnt = 0; //job count
            predicted_exec_time  = exec_time_arr[job_cnt];
            moment_timing_print(0); //moment_start
            
            start_timing();
            set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            end_timing();
            dvfs_time = print_dvfs_timing();
            
            moment_timing_print(1); //moment_start
            job_cnt++;
        #elif PID_EN /* CASE 6 */
            moment_timing_print(0); //moment_start
            
            start_timing();
            predicted_exec_time = pid_controller(exec_time); //pid == slice
            end_timing();
            slice_time = print_slice_timing();
            
            start_timing();
            set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            end_timing();
            dvfs_time = print_dvfs_timing();
            
            moment_timing_print(1); //moment_start
        #endif

//---------------------modified by TJSong----------------------//

          start_timing();

          //sha_stream(&sha_info, fin);
          sha_stream(&sha_info, file_buffer, flen);
          sha_print(&sha_info);

          end_timing();
//---------------------modified by TJSong----------------------//
        exec_time = exec_timing();
        int delay_time = 0;

        #if GET_PREDICT /* CASE 0 */
            print_exec_time(exec_time);
        #elif GET_DEADLINE /* CASE 1 */
            print_exec_time(exec_time);
        #elif GET_OVERHEAD /* CASE 2 */
            //nothing
        #else /* CASE 3,4,5 and 6 */
            if(DELAY_EN && ((delay_time = DEADLINE_TIME - exec_time - slice_time - dvfs_time) > 0)){
                start_timing();
                usleep(delay_time);
                end_timing();
                delay_time = exec_timing();
            }else
                delay_time = 0;
        moment_timing_print(2); //moment_end
        print_exec_time(exec_time);
        print_total_time(exec_time + slice_time + dvfs_time + delay_time);
        #endif
        fclose_all();//TJSong

        // Write out predicted time & print out frequency used
        print_predicted_time(predicted_exec_time);
        print_freq(); 

//---------------------modified by TJSong----------------------//


        }
      }
    }
    return(0);
}
