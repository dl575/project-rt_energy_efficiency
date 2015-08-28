/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"
#include "timing.h"
#include <unistd.h>

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
    #if !CVX_EN //conservative
        exec_time = 0;
    #else //cvx    
        if(CVX_COEFF == 100)
            exec_time = 39.731974*loop_counter[0] + 39.977368*loop_counter[1] + 0.000885*loop_counter[3] + 0.000132*loop_counter[4] + 0.000036*loop_counter[5] + 0.000087*loop_counter[6] + 0.000087*loop_counter[7] + 0.000087*loop_counter[8] + 0.000087*loop_counter[9] + -2.631229*loop_counter[10] + -0.328817*loop_counter[11] + -0.116261*loop_counter[12] + -0.278069*loop_counter[13] + -0.278069*loop_counter[14] + -0.278069*loop_counter[15] + -0.278069*loop_counter[16] + 4.160141*loop_counter[17] + 1.461587*loop_counter[18] + 3.480920*loop_counter[19] + 3.480920*loop_counter[20] + 3.480920*loop_counter[21] + 3.480920*loop_counter[22] + 32.918977;
    #endif
#else
    #if !CVX_EN //conservative
        exec_time = 80.605400*loop_counter[1] + 0.887891*loop_counter[3] + 15.467700*loop_counter[10] + 30.718300*loop_counter[17] + 0.000000;
    #else //cvx    
        if(CVX_COEFF == 10)
            exec_time = -23.689941*loop_counter[0] + -23.793744*loop_counter[1] + 0.132241*loop_counter[3] + 0.016516*loop_counter[4] + 0.005840*loop_counter[5] + 0.013962*loop_counter[6] + 0.013962*loop_counter[7] + 0.013962*loop_counter[8] + 0.013962*loop_counter[9] + 23.391490*loop_counter[10] + 2.923825*loop_counter[11] + 1.033574*loop_counter[12] + 2.473365*loop_counter[13] + 2.473365*loop_counter[14] + 2.473365*loop_counter[15] + 2.473365*loop_counter[16] + 0.974851*loop_counter[17] + 0.344855*loop_counter[18] + 0.824862*loop_counter[19] + 0.824862*loop_counter[20] + 0.824862*loop_counter[21] + 0.824862*loop_counter[22] + 7.801028;
        else if(CVX_COEFF == 50)
            exec_time = 40.301451*loop_counter[0] + 40.303939*loop_counter[1] + 0.062243*loop_counter[3] + 0.007771*loop_counter[4] + 0.002748*loop_counter[5] + 0.006568*loop_counter[6] + 0.006568*loop_counter[7] + 0.006568*loop_counter[8] + 0.006568*loop_counter[9] + 1.083027*loop_counter[10] + 0.135339*loop_counter[11] + 0.047805*loop_counter[12] + 0.114497*loop_counter[13] + 0.114497*loop_counter[14] + 0.114497*loop_counter[15] + 0.114497*loop_counter[16] + 3.947360*loop_counter[17] + 1.395664*loop_counter[18] + 3.339157*loop_counter[19] + 3.339157*loop_counter[20] + 3.339157*loop_counter[21] + 3.339157*loop_counter[22] + 31.578068;
        else if(CVX_COEFF == 100)
            exec_time = 40.301967*loop_counter[0] + 40.303426*loop_counter[1] + 0.062132*loop_counter[3] + 0.007766*loop_counter[4] + 0.002747*loop_counter[5] + 0.006571*loop_counter[6] + 0.006571*loop_counter[7] + 0.006571*loop_counter[8] + 0.006571*loop_counter[9] + 1.082202*loop_counter[10] + 0.135301*loop_counter[11] + 0.047775*loop_counter[12] + 0.114539*loop_counter[13] + 0.114539*loop_counter[14] + 0.114539*loop_counter[15] + 0.114539*loop_counter[16] + 3.947418*loop_counter[17] + 1.395629*loop_counter[18] + 3.339139*loop_counter[19] + 3.339139*loop_counter[20] + 3.339139*loop_counter[21] + 3.339139*loop_counter[22] + 31.580287;
    #endif
#endif 
   return exec_time;
  }
}

int main(int argc, char **argv)
{
    FILE *fin;
    SHA_INFO sha_info;

//---------------------modified by TJSong----------------------//
    static int jump = 0;
    int exec_time = 0;
    int pid = getpid();
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
            CASE 7 = running on proactive DVFS
        */
        #if GET_PREDICT /* CASE 0 */
            predicted_exec_time = sha_stream_slice(&sha_info, file_buffer, flen);
        #elif GET_DEADLINE /* CASE 1 */
            moment_timing_print(0); //moment_start
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
        #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN /* CASE 3 */
            //slice_time=0; dvfs_time=0;
            predicted_exec_time = sha_stream_slice(&sha_info, file_buffer, flen);
            moment_timing_print(0); //moment_start
        #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN /* CASE 4 */
            moment_timing_print(0); //moment_start
            
            start_timing();
            predicted_exec_time = sha_stream_slice(&sha_info, file_buffer, flen);
            end_timing();
            slice_time = print_slice_timing();
            
            start_timing();
            #if OVERHEAD_EN //with overhead
                set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            #else //without overhead
                set_freq(predicted_exec_time, 0, DEADLINE_TIME, 0); //do dvfs
            #endif
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
        #elif PROACTIVE_EN /* CASE 4 */
            static int job_number = 0; //job count
            moment_timing_print(0); //moment_start
           
            /*char cmd[100];
            if(job_number %2 ==0){
                printf("job_number is %d\n", job_number);
                sprintf(cmd, "taskset -p %s %d", "0xf0", pid);
                fflush(stdout);
                system(cmd);
            }else{
                printf("job_number is %d\n", job_number);
                sprintf(cmd, "taskset -p %s %d", "0x0f", pid);
                fflush(stdout);
                system(cmd);
            }*/
 
            start_timing();
            jump = set_freq_multiple_hetero(job_number, DEADLINE_TIME, pid); //do dvfs
            end_timing();
            dvfs_time = print_dvfs_timing();
            
            moment_timing_print(1); //moment_start
            job_number++;
        #endif

//---------------------modified by TJSong----------------------//

          start_timing();
          //print_start_emperature();

          //sha_stream(&sha_info, fin);
          sha_stream(&sha_info, file_buffer, flen);
          sha_print(&sha_info);

          //print_end_temperature();
          end_timing();
//---------------------modified by TJSong----------------------//
        exec_time = exec_timing();
        int delay_time = 0;

        #if GET_PREDICT /* CASE 0 */
            print_exec_time(exec_time);
        #elif GET_DEADLINE /* CASE 1 */
            print_exec_time(exec_time);
            moment_timing_print(2); //moment_end
        #elif GET_OVERHEAD /* CASE 2 */
            //nothing
        #else /* CASE 3,4,5,6 and 7 */
            if(DELAY_EN && jump == 0 && ((delay_time = DEADLINE_TIME - exec_time - slice_time - dvfs_time) > 0)){
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

