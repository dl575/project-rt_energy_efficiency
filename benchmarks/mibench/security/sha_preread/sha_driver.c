/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"
#include "timing.h"
#include <unistd.h>

#if !HETERO_EN
struct slice_return sha_stream_slice(SHA_INFO *sha_info, char *file_buffer, 
    int flen, llsp_t *restrict solver)
#elif HETERO_EN
struct slice_return sha_stream_slice(SHA_INFO *sha_info, char *file_buffer, 
    int flen, llsp_t *restrict solver_big, llsp_t *restrict solver_little)
#endif
{
  int loop_counter[23] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int i;
  {
    sha_info->count_lo = 0L;
    //return0:
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

          //return3:
          ;

        }
        count_rename1 -= 64;
      }

      //return1:
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

        //return4:
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

      //return5:
      ;

    }
    //return2:
    ;

  }
  {
    //print_loop_counter:
    ;
#if GET_PREDICT || DEBUG_EN
    //23
    print_array(loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]));
#endif
  }
  {
    //predict_exec_time:
    ;
    struct slice_return exec_time;
    exec_time.big = exec_time.little = 0; //initialize
#if !ONLINE_EN
  #if !CVX_EN //off-line training with conservative
    #if ARCH_ARM
      exec_time.little = 1.338970*loop_counter[0] + 1.347246*loop_counter[1] + 0.109213*loop_counter[3] + 0.013651*loop_counter[4] + 0.004827*loop_counter[5] + 0.011547*loop_counter[6] + 0.011547*loop_counter[7] + 0.011547*loop_counter[8] + 0.011547*loop_counter[9] + 3.971997*loop_counter[10] + 0.496534*loop_counter[11] + 0.175529*loop_counter[12] + 0.420008*loop_counter[13] + 0.420008*loop_counter[14] + 0.420008*loop_counter[15] + 0.420008*loop_counter[16] + 0.410735*loop_counter[17] + 0.145201*loop_counter[18] + 0.348669*loop_counter[19] + 0.348669*loop_counter[20] + 0.348669*loop_counter[21] + 0.348669*loop_counter[22] + 3.286883;
    #elif ARCH_X86
      exec_time.little = 9.547252*loop_counter[0] + 9.663151*loop_counter[1] +
      0.056890*loop_counter[3] + 0.003556*loop_counter[4] +
      0.000889*loop_counter[5] + 0.002845*loop_counter[6] +
      0.002845*loop_counter[7] + 0.002845*loop_counter[8] +
      0.002845*loop_counter[9] + 0.000007*loop_counter[11] +
      0.036845*loop_counter[12] + 0.000009*loop_counter[13] +
      0.000009*loop_counter[14] + 0.000009*loop_counter[15] +
      0.000009*loop_counter[16] + -0.392372*loop_counter[17] +
      -0.098093*loop_counter[18] + -0.313897*loop_counter[19] +
      -0.313897*loop_counter[20] + -0.313897*loop_counter[21] +
      -0.313897*loop_counter[22] + -0.000020;
    #endif
  #else //off-line training with cvx    
    #if ARCH_ARM
      /*exec_time.little = 53.525647*loop_counter[0] +
       * 53.967833*loop_counter[1] + 0.056076*loop_counter[3] +
       * 0.007013*loop_counter[4] + 0.002483*loop_counter[5] +
       * 0.005935*loop_counter[6] + 0.005935*loop_counter[7] +
       * 0.005935*loop_counter[8] + 0.005935*loop_counter[9] +
       * -2.424692*loop_counter[10] + -0.302937*loop_counter[11] +
       * -0.107123*loop_counter[12] + -0.256198*loop_counter[13] +
       * -0.256198*loop_counter[14] + -0.256198*loop_counter[15] +
       * -0.256198*loop_counter[16] + 0.796433*loop_counter[17] +
       * 0.281992*loop_counter[18] + 0.677867*loop_counter[19] +
       * 0.677867*loop_counter[20] + 0.677867*loop_counter[21] +
       * 0.677867*loop_counter[22] + 6.380048;*/
      exec_time.little = -5.066004*loop_counter[0] + -5.198613*loop_counter[1] + 0.210721*loop_counter[3] + 0.013170*loop_counter[4] + 0.003293*loop_counter[5] + 0.010537*loop_counter[6] + 0.010537*loop_counter[7] + 0.010537*loop_counter[8] + 0.010537*loop_counter[9] + -0.000431*loop_counter[10] + -0.006902*loop_counter[11] + -0.027606*loop_counter[12] + -0.008628*loop_counter[13] + -0.008628*loop_counter[14] + -0.008628*loop_counter[15] + -0.008628*loop_counter[16] + 0.000132*loop_counter[17] + 0.197178*loop_counter[18] + 0.000235*loop_counter[19] + 0.000235*loop_counter[20] + 0.000235*loop_counter[21] + 0.000235*loop_counter[22] + 0.000008;
      exec_time.big = -13.891408*loop_counter[0] + -14.082207*loop_counter[1] +
        0.122227*loop_counter[3] + 0.007639*loop_counter[4] +
        0.001910*loop_counter[5] + 0.006111*loop_counter[6] +
        0.006111*loop_counter[7] + 0.006111*loop_counter[8] +
        0.006111*loop_counter[9] + 0.000025*loop_counter[10] +
        0.000392*loop_counter[11] + 0.001569*loop_counter[12] +
        0.000490*loop_counter[13] + 0.000490*loop_counter[14] +
        0.000490*loop_counter[15] + 0.000490*loop_counter[16] +
        -0.000027*loop_counter[17] + -0.026362*loop_counter[18] +
        -0.000030*loop_counter[19] + -0.000030*loop_counter[20] +
        -0.000030*loop_counter[21] + -0.000030*loop_counter[22] + -0.000002;

    #elif ARCH_X86
    #endif
  #endif
#elif ONLINE_EN
  #if !HETERO_EN
    #if CORE //on-line training on big core
      exec_time.big    = get_predicted_time(TYPE_PREDICT, solver, loop_counter,
          sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    #else //on-line training on little core
      exec_time.little = get_predicted_time(TYPE_PREDICT, solver, loop_counter,
          sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    #endif
  #elif HETERO_EN
    exec_time.big    = get_predicted_time_big(TYPE_PREDICT, solver_big,
        loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    exec_time.little = get_predicted_time_little(TYPE_PREDICT, solver_little,
        loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
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
  _INIT_();
  static int current_core = CORE;
#if HETERO_EN
  static int is_stable_big = 0; //0: not stable
  static int is_stable_little = 0; //0: not stable
  int pid = getpid();
  llsp_t *solver_big = llsp_new(N_FEATURE + 1);
  llsp_t *solver_little = llsp_new(N_FEATURE + 1);
#elif !HETERO_EN
  llsp_t *solver = llsp_new(N_FEATURE + 1);
#endif
  //---------------------modified by TJSong----------------------//

  if(argc < 2){
    printf("stdin read not currently supported for slicing. Please implement.\n");
    exit(1);
    /*
       fin = stdin;
       sha_stream(&sha_info, fin);
       sha_print(&sha_info);
     */
  }else{
    while(--argc){
      fin = fopen(*(++argv), "rb");
      if(fin == NULL){
        printf("error opening %s for reading\n", *argv);
        exit(1);
      }else{
        // Get file length
        int flen;
        fseek(fin, 0, SEEK_END);
        fgetpos(fin, (fpos_t*)&flen);
        fseek(fin, 0, SEEK_SET);
        // Allocate buffer
        char *file_buffer = malloc(sizeof(char) * (flen + 1));
        // Read file into buffer
        size_t newLen = fread(file_buffer, sizeof(char), flen, fin);
        if (newLen == 0){
          printf("Error reading file\n");
          exit(1);
        }else{
          file_buffer[++newLen] = '\0';
        }
        fclose(fin);

        //---------------------modified by TJSong----------------------//
        fopen_all(); //fopen for frequnecy file
        print_deadline(DEADLINE_TIME); //print deadline 
        //---------------------modified by TJSong----------------------//

        //---------------------modified by TJSong----------------------//
        // Perform slicing and prediction
        struct slice_return predicted_exec_time;
        predicted_exec_time.big = 0;
        predicted_exec_time.little = 0;
        /*
          CASE 0 = to get prediction equation
          CASE 3 = running on default linux governors
          CASE 4 = running on our prediction
          CASE 6 = running on pid
        */
        #if GET_PREDICT /* CASE 0 */
          predicted_exec_time = _SLICE_();
        #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN /* CASE 3 */
          predicted_exec_time = _SLICE_();
          moment_timing_print(0); //moment_start
        #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN /* CASE 4 */
          moment_timing_print(0); //moment_start
          
          start_timing();
          predicted_exec_time = _SLICE_();
          end_timing();
          slice_time = print_slice_timing();
          
          start_timing();
          #if OVERHEAD_EN //with overhead
            #if HETERO_EN
              current_core = set_freq_hetero(predicted_exec_time.big, 
                  predicted_exec_time.little, slice_time, DEADLINE_TIME, 
                  AVG_DVFS_TIME, pid, is_stable_big, is_stable_little); //do dvfs
            #else
              #if CORE
                set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
              #else
                set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
              #endif
            #endif
          #endif
          end_timing();
          dvfs_time = print_dvfs_timing();

          moment_timing_print(1); //moment_start
        #elif ORACLE_EN /* CASE 5 */
        #elif PID_EN /* CASE 6 */
          moment_timing_print(0); //moment_start
          
          start_timing();
          predicted_exec_time = pid_controller(exec_time); //pid == slice
          end_timing();
          slice_time = print_slice_timing();
          
          start_timing();
          #if CORE
            set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
          #else
            set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
          #endif
          end_timing();
          dvfs_time = print_dvfs_timing();
          
          moment_timing_print(1); //moment_start
        #elif PROACTIVE_EN /* CASE 7 */
        #endif

        //---------------------modified by TJSong----------------------//
        start_timing();

        //sha_stream(&sha_info, fin);
        sha_stream(&sha_info, file_buffer, flen);
        sha_print(&sha_info);

        end_timing();
        //---------------------modified by TJSong----------------------//
        
        _DEFINE_TIME_();

        #if IDLE_EN
          additional_dvfs_times =
            dvfs_table[cur_freq/100000-2][MIN_FREQ/100000-2] +
            dvfs_table[MIN_FREQ/100000-2][cur_freq/100000-2];
        #endif

        #if ONLINE_EN /* CASE 0, 2, 3 and 4 */
          #if GET_PREDICT || GET_OVERHEAD \
                || (!PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN) \
                || (!PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN) 
            start_timing();
            #if !HETERO_EN
              (void)get_predicted_time(TYPE_SOLVE, solver, NULL, 0, exec_time,
                cur_freq);
            #elif HETERO_EN
              if     (current_core == 1)
                is_stable_big    = get_predicted_time_big   (TYPE_SOLVE, 
                    solver_big,    NULL, 0, exec_time, cur_freq);
              else if(current_core == 0)
                is_stable_little = get_predicted_time_little(TYPE_SOLVE, 
                    solver_little, NULL, 0, exec_time, cur_freq);
            #endif
            end_timing();
            update_time = exec_timing();
          #endif
        #endif

        _DELAY_();

        _PRINT_INFO_();
        
        fclose_all();//TJSong
        //---------------------modified by TJSong----------------------//


      }
    }
  }
  return(0);
}

