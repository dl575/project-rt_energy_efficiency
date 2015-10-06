
 /*
   -----------------------------------------------------------------------
   Copyright (c) 2001 Dr Brian Gladman <brg@gladman.uk.net>, Worcester, UK
   
   TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   This software is provided 'as is' with no guarantees of correctness or
   fitness for purpose.
   -----------------------------------------------------------------------
 */

/* Example of the use of the AES (Rijndael) algorithm for file  */
/* encryption.  Note that this is an example application, it is */
/* not intended for real operational use.  The Command line is: */
/*                                                              */
/* aesxam input_file_name output_file_name [D|E] hexadecimalkey */
/*                                                              */
/* where E gives encryption and D decryption of the input file  */
/* into the output file using the given hexadecimal key string  */
/* The later is a hexadecimal sequence of 32, 48 or 64 digits   */
/* Examples to encrypt or decrypt aes.c into aes.enc are:       */
/*                                                              */
/* aesxam file.c file.enc E 0123456789abcdeffedcba9876543210    */
/*                                                              */
/* aesxam file.enc file2.c D 0123456789abcdeffedcba9876543210   */
/*                                                              */
/* which should return a file 'file2.c' identical to 'file.c'   */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "aes.h"

#include "timing.h"
#include "my_common.h"

/* A Pseudo Random Number Generator (PRNG) used for the     */
/* Initialisation Vector. The PRNG is George Marsaglia's    */
/* Multiply-With-Carry (MWC) PRNG that concatenates two     */
/* 16-bit MWC generators:                                   */
/*     x(n)=36969 * x(n-1) + carry mod 2^16                 */ 
/*     y(n)=18000 * y(n-1) + carry mod 2^16                 */
/* to produce a combined PRNG with a period of about 2^60.  */  
/* The Pentium cycle counter is used to initialise it. This */
/* is crude but the IV does not need to be secret.          */
 
/* void cycles(unsigned long *rtn)     */
/* {                           // read the Pentium Time Stamp Counter */
/*     __asm */
/*     { */
/*     _emit   0x0f            // complete pending operations */
/*     _emit   0xa2 */
/*     _emit   0x0f            // read time stamp counter */
/*     _emit   0x31 */
/*     mov     ebx,rtn */
/*     mov     [ebx],eax */
/*     mov     [ebx+4],edx */
/*     _emit   0x0f            // complete pending operations */
/*     _emit   0xa2 */
/*     } */
/* } */

#define RAND(a,b) (((a = 36969 * (a & 65535) + (a >> 16)) << 16) + (b = 18000 * (b & 65535) + (b >> 16))  )

void fillrand(char *buf, int len, int reset)
{   static unsigned long a[2], mt = 1, count = 4;
    static char          r[4];
    int                  i;

    if (reset) {
      mt = 1;
      count = 4;
    } else {
      if(mt) { 
        mt = 0; 
        /*cycles(a);*/
        a[0]=0xeaf3;
        a[1]=0x35fe;
      }

      for(i = 0; i < len; ++i)
      {
        if(count == 4)
        {
          *(unsigned long*)r = RAND(a[0], a[1]);
          count = 0;
        }

        buf[i] = r[count++];
      }
    }
}    

int encfile(FILE *fout, aes *ctx, char* fn, char *file_buffer, int flen)
{   char            inbuf[16], outbuf[16];
    //int          flen;
    unsigned long   i=0, l=0;

    fillrand(outbuf, 16, 0);           /* set an IV for CBC mode           */
    //fseek(fin, 0, SEEK_END);        /* get the length of the file       */
    //fgetpos(fin, &flen);            /* and then reset to start          */
    //fseek(fin, 0, SEEK_SET);        
    fwrite(outbuf, 1, 16, fout);    /* write the IV to the output       */
    fillrand(inbuf, 1, 0);             /* make top 4 bits of a byte random */
    l = 15;                         /* and store the length of the last */
                                    /* block in the lower 4 bits        */
    inbuf[0] = ((char)flen & 15) | (inbuf[0] & ~15);

    //while(!feof(fin))               /* loop to encrypt the input file   */
    int j;
    for (j = 0; j < flen; j += 16)
    {                               /* input 1st 16 bytes to buf[1..16] */
        //i = fread(inbuf + 16 - l, 1, l, fin);  /*  on 1st round byte[0] */
                                               /* is the length code    */
        //if(i < l) break;            /* if end of the input file reached */
        i = flen - j;
        if (i < l) {
            int k;
            memcpy(inbuf + 16 - l, file_buffer + j, i); 
            break;
        }
        memcpy(inbuf + 16 - l, file_buffer + j, l); 

        for(i = 0; i < 16; ++i)         /* xor in previous cipher text  */
            inbuf[i] ^= outbuf[i]; 

        _encrypt(inbuf, outbuf, ctx);    /* and do the encryption        */

        if(fwrite(outbuf, 1, 16, fout) != 16)
        {
            printf("Error writing to output file: %s\n", fn);
            return -7;
        }
                                    /* in all but first round read 16   */
        l = 16;                     /* bytes into the buffer            */
    }

    /* except for files of length less than two blocks we now have one  */
    /* byte from the previous block and 'i' bytes from the current one  */
    /* to encrypt and 15 - i empty buffer positions. For files of less  */
    /* than two blocks (0 or 1) we have i + 1 bytes and 14 - i empty    */
    /* buffer position to set to zero since the 'count' byte is extra   */

    if(l == 15)                         /* adjust for extra byte in the */
        ++i;                            /* in the first block           */

    if(i)                               /* if bytes remain to be output */
    {
        while(i < 16)                   /* clear empty buffer positions */
            inbuf[i++] = 0;
     
        for(i = 0; i < 16; ++i)         /* xor in previous cipher text  */
            inbuf[i] ^= outbuf[i]; 

        _encrypt(inbuf, outbuf, ctx);    /* encrypt and output it        */

        if(fwrite(outbuf, 1, 16, fout) != 16)
        {
            printf("Error writing to output file: %s\n", fn);
            return -8;
        }
    }
        
    return 0;
}

int decfile(FILE *fin, FILE *fout, aes *ctx, char* ifn, char* ofn)
{   char    inbuf1[16], inbuf2[16], outbuf[16], *bp1, *bp2, *tp;
    int     i, l, flen;

    if(fread(inbuf1, 1, 16, fin) != 16)  /* read Initialisation Vector   */
    {
        printf("Error reading from input file: %s\n", ifn);
        return 9;
    }

    i = fread(inbuf2, 1, 16, fin);  /* read 1st encrypted file block    */

    if(i && i != 16)
    {
        printf("\nThe input file is corrupt");
        return -10;
    }

    decrypt(inbuf2, outbuf, ctx);   /* decrypt it                       */

    for(i = 0; i < 16; ++i)         /* xor with previous input          */
        outbuf[i] ^= inbuf1[i];

    flen = outbuf[0] & 15;  /* recover length of the last block and set */
    l = 15;                 /* the count of valid bytes in block to 15  */                              
    bp1 = inbuf1;           /* set up pointers to two input buffers     */
    bp2 = inbuf2;

    while(1)
    {
        i = fread(bp1, 1, 16, fin);     /* read next encrypted block    */
                                        /* to first input buffer        */
        if(i != 16)         /* no more bytes in input - the decrypted   */
            break;          /* partial final buffer needs to be output  */

        /* if a block has been read the previous block must have been   */
        /* full lnegth so we can now write it out                       */
         
        if(fwrite(outbuf + 16 - l, 1, l, fout) != (unsigned long)l)
        {
            printf("Error writing to output file: %s\n", ofn);
            return -11;
        }

        decrypt(bp1, outbuf, ctx);  /* decrypt the new input block and  */

        for(i = 0; i < 16; ++i)     /* xor it with previous input block */
            outbuf[i] ^= bp2[i];
        
        /* set byte count to 16 and swap buffer pointers                */

        l = i; tp = bp1, bp1 = bp2, bp2 = tp;
    }

    /* we have now output 16 * n + 15 bytes of the file with any left   */
    /* in outbuf waiting to be output. If x bytes remain to be written, */
    /* we know that (16 * n + x + 15) % 16 = flen, giving x = flen + 1  */
    /* But we must also remember that the first block is offset by one  */
    /* in the buffer - we use the fact that l = 15 rather than 16 here  */  

    l = (l == 15 ? 1 : 0); flen += 1 - l;

    if(flen)
        if(fwrite(outbuf + l, 1, flen, fout) != (unsigned long)flen)
        {
            printf("Error writing to output file: %s\n", ofn);
            return -12;
        }

    return 0;
}

struct slice_return encfile_slice(FILE *fout, aes *ctx, char *fn, char *file_buffer, int flen)
{
  int loop_counter[23] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char inbuf[16];
  char outbuf[16];
  unsigned long i = 0;
  unsigned long l = 0;
  {
    int len_rename0 = 16;
    unsigned long mt_rename0 = 1;
    unsigned long count_rename0 = 4;
    char r_rename0[4];
    int i_rename0;
    if (mt_rename0)
    {
      loop_counter[0]++;
      mt_rename0 = 0;
    }

    for (i_rename0 = 0; i_rename0 < len_rename0; ++i_rename0)
    {
      loop_counter[1]++;
      if (count_rename0 == 4)
      {
        loop_counter[2]++;
        count_rename0 = 0;
      }

      outbuf[i_rename0] = r_rename0[count_rename0++];
    }

    return0:
    ;

  }
  {
    int len_rename1 = 1;
    unsigned long mt_rename1 = 1;
    unsigned long count_rename1 = 4;
    char r_rename1[4];
    int i_rename1;
    if (mt_rename1)
    {
      loop_counter[3]++;
      mt_rename1 = 0;
    }

    for (i_rename1 = 0; i_rename1 < len_rename1; ++i_rename1)
    {
      loop_counter[4]++;
      if (count_rename1 == 4)
      {
        loop_counter[5]++;
        count_rename1 = 0;
      }

      inbuf[i_rename1] = r_rename1[count_rename1++];
    }

    return1:
    ;

  }
  l = 15;
  inbuf[0] = (((char) flen) & 15) | (inbuf[0] & (~15));
  int j;
  for (j = 0; j < flen; j += 16)
  {
    loop_counter[6]++;
    i = flen - j;
    if (i < l)
    {
      loop_counter[7]++;
      break;
    }

    for (i = 0; i < 16; ++i)
    {
      loop_counter[8]++;
      inbuf[i] ^= outbuf[i];
    }

    {
      if (!(ctx->mode & 0x01))
      {
        loop_counter[9]++;
        goto return2;
      }

      switch (ctx->Nrnd)
      {
        case 14:
          loop_counter[10]++;

        case 12:
          loop_counter[11]++;

        case 10:
          loop_counter[12]++;

      }

      {
        goto return2;
      }
      return2:
      ;

    }
    /*
    {
      size_t fwrite_result0;
      fwrite_result0 = fwrite(outbuf, 1, 16, fout);
      if (fwrite_result0 != 16)
      {
        loop_counter[13]++;
        {
          goto print_loop_counter;
        }
      }

    }
    */
    l = 16;
  }

  if (l == 15)
  {
    loop_counter[14]++;
    ++i;
  }

  if (i)
  {
    loop_counter[15]++;
    while (i < 16)
    {
      loop_counter[16]++;
      inbuf[i++] = 0;
    }

    for (i = 0; i < 16; ++i)
    {
      loop_counter[17]++;
      inbuf[i] ^= outbuf[i];
    }

    {
      if (!(ctx->mode & 0x01))
      {
        loop_counter[18]++;
        goto return3;
      }

      switch (ctx->Nrnd)
      {
        case 14:
          loop_counter[19]++;

        case 12:
          loop_counter[20]++;

        case 10:
          loop_counter[21]++;

      }

      {
        goto return3;
      }
      return3:
      ;

    }
    {
      size_t fwrite_result0;
      fwrite_result0 = fwrite(outbuf, 1, 16, fout);
      if (fwrite_result0 != 16)
      {
        loop_counter[22]++;
        {
          goto print_loop_counter;
        }
      }

    }
  }

  {
    goto print_loop_counter;
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
    struct slice_return exec_time;
    #if !CVX_EN //conservative
        exec_time.big = 0;
        exec_time.little = 2739.740000*loop_counter[0] + 14435.700000*loop_counter[7] + 0.089650*loop_counter[8] + -1039.580000*loop_counter[16] + 0.000000;
    #else // cvx
        if(CVX_COEFF == 100){
            exec_time.big = -16.998409*loop_counter[0] + -2.122975*loop_counter[1] + -6.028204*loop_counter[2] + -16.998321*loop_counter[3] + -16.998321*loop_counter[4] + -16.998321*loop_counter[5] + 0.117920*loop_counter[6] + 140.525487*loop_counter[7] + 0.014736*loop_counter[8] + 0.117990*loop_counter[10] + 0.117990*loop_counter[11] + 0.117990*loop_counter[12] + -16.992676*loop_counter[15] + 13.944916*loop_counter[16] + -2.121213*loop_counter[17] + -16.992588*loop_counter[19] + -16.992588*loop_counter[20] + -16.992588*loop_counter[21] + -16.992574;
			exec_time.little = 48.166445*loop_counter[0] + 6.021475*loop_counter[1] + 17.031604*loop_counter[2] + 48.166383*loop_counter[3] + 48.166383*loop_counter[4] + 48.166383*loop_counter[5] + 0.308425*loop_counter[6] + -363.743240*loop_counter[7] + 0.038558*loop_counter[8] + 0.308473*loop_counter[10] + 0.308473*loop_counter[11] + 0.308473*loop_counter[12] + 48.166277*loop_counter[15] + 28.759982*loop_counter[16] + 6.021270*loop_counter[17] + 48.166489*loop_counter[19] + 48.166489*loop_counter[20] + 48.166489*loop_counter[21] + 48.166445;

        }
    #endif 
    return exec_time;
  }
}

int main(int argc, char *argv[])
{   
    int err=0;

    int argv_i;

//---------------------modified by TJSong----------------------//
    int exec_time = 0;
    static int jump = 0;
    int pid = getpid();
    if(check_define()==ERROR_DEFINE){
        printf("%s", "DEFINE ERROR!!\n");
        return ERROR_DEFINE;
    }
//---------------------modified by TJSong----------------------//


#define N_ARGS 4
    for (argv_i = 0; argv_i + N_ARGS < argc; argv_i += N_ARGS) {
        // Reset static variables
        fillrand(NULL, 0, 1);

        FILE    *fin = 0, *fout = 0;
        char    *cp, ch, key[32];
        int     i=0, by=0, key_len=0;
        aes     ctx[1];

        /*
        if(argc != 5 || (toupper(*argv[3]) != 'D' && toupper(*argv[3]) != 'E'))
        {
            printf("usage: rijndael in_filename out_filename [d/e] key_in_hex\n"); 
            err = -1; goto exit;
        }
        */

        cp = argv[argv_i + 4];   /* this is a pointer to the hexadecimal key digits  */
        i = 0;          /* this is a count for the input digits processed   */
        
        while(i < 64 && *cp)    /* the maximum key length is 32 bytes and   */
        {                       /* hence at most 64 hexadecimal digits      */
            ch = toupper(*cp++);            /* process a hexadecimal digit  */
            if(ch >= '0' && ch <= '9')
                by = (by << 4) + ch - '0';
            else if(ch >= 'A' && ch <= 'F')
                by = (by << 4) + ch - 'A' + 10;
            else                            /* error if not hexadecimal     */
            {
                printf("key must be in hexadecimal notation\n"); 
                err = -2; goto exit;
            }
            
            /* store a key byte for each pair of hexadecimal digits         */
            if(i++ & 1) 
                key[i / 2 - 1] = by & 0xff; 
        }

        if(*cp)
        {
            printf("The key value is too long\n"); 
            err = -3; goto exit;
        }
        else if(i < 32 || (i & 15))
        {
            printf("The key length must be 32, 48 or 64 hexadecimal digits\n");
            err = -4; goto exit;
        }

        key_len = i / 2;

        if(!(fin = fopen(argv[argv_i + 1], "rb")))   /* try to open the input file */
        {
            printf("The input file: %s could not be opened\n", argv[argv_i + 1]); 
            err = -5; goto exit;
        }

        if(!(fout = fopen(argv[argv_i + 2], "wb")))  /* try to open the output file */
        {
            printf("The output file: %s could not be opened\n", argv[argv_i + 1]); 
            err = -6; goto exit;
        }

        if(toupper(*argv[argv_i + 3]) == 'E')
        {                           /* encryption in Cipher Block Chaining mode */
            set_key(key, key_len, enc, ctx);

            // Get file length
            int flen;
            fseek(fin, 0, SEEK_END);        /* get the length of the file       */
            fgetpos(fin, &flen);            /* and then reset to start          */
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
            CASE 1 = to get execution deadline
            CASE 2 = to get overhead deadline
            CASE 3 = running on default linux governors
            CASE 4 = running on our prediction
            CASE 5 = running on oracle
            CASE 6 = running on pid
            CASE 7 = running on proactive DVFS
        */
        #if GET_PREDICT /* CASE 0 */
            predicted_exec_time = encfile_slice(fout, ctx, argv[argv_i + 1], file_buffer, flen);
        #elif GET_DEADLINE /* CASE 1 */
            moment_timing_print(0); //moment_start
            //nothing
        #elif GET_OVERHEAD /* CASE 2 */
            start_timing();
            predicted_exec_time = encfile_slice(fout, ctx, argv[argv_i + 1], file_buffer, flen);
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
        #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN /* CASE 3 */
            //slice_time=0; dvfs_time=0;
            predicted_exec_time = encfile_slice(fout, ctx, argv[argv_i + 1], file_buffer, flen);
            moment_timing_print(0); //moment_start
        #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN /* CASE 4 */
            moment_timing_print(0); //moment_start
            
            start_timing();
            predicted_exec_time = encfile_slice(fout, ctx, argv[argv_i + 1], file_buffer, flen);
            end_timing();
            slice_time = print_slice_timing();
            
            start_timing();
            #if OVERHEAD_EN //with overhead
                #if HETERO_EN
                    set_freq_hetero(predicted_exec_time.big, predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME, pid); //do dvfs
                #else
                    #if CORE
                        set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
                    #else
                        set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
                    #endif
                #endif
            #else //without overhead
                #if HETERO_EN
                    set_freq_hetero(predicted_exec_time.big, predicted_exec_time.little, 0, DEADLINE_TIME, 0, pid); //do dvfs
                #else
                    #if CORE
                        set_freq(predicted_exec_time.big, 0, DEADLINE_TIME, 0); //do dvfs
                    #else
                        set_freq(predicted_exec_time.little, 0, DEADLINE_TIME, 0); //do dvfs
                    #endif
                #endif
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
            #if CORE
                set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            #else
                set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            #endif
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
            #if CORE
                set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            #else
                set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
            #endif
            end_timing();
            dvfs_time = print_dvfs_timing();
            
            moment_timing_print(1); //moment_start
        #elif PROACTIVE_EN /* CASE 4 */
            static int job_number = 0; //job count
            moment_timing_print(0); //moment_start
          
            start_timing();
            //Now, let's assume no slice time like ORACLE
            end_timing();
            slice_time = print_slice_timing();
 
            start_timing();
            #if HETERO_EN 
                jump = set_freq_multiple_hetero(job_number, DEADLINE_TIME, pid); //do dvfs
            #elif !HETERO_EN
                jump = set_freq_multiple(job_number, DEADLINE_TIME); //do dvfs
            #endif
            end_timing();
            dvfs_time = print_dvfs_timing();
            
            moment_timing_print(1); //moment_start
            job_number++;
        #endif
    //---------------------modified by TJSong----------------------//

            start_timing();
            //print_start_temperature();
            // Run encryption
            err = encfile(fout, ctx, argv[argv_i + 1], file_buffer, flen);
            //print_end_temperature();
            end_timing();

            free(file_buffer);

    //---------------------modified by TJSong----------------------//
        exec_time = exec_timing();
        int cur_freq = print_freq(); 
        int delay_time = 0;
        int actual_delay_time = 0;

        #if GET_PREDICT /* CASE 0 */
            print_exec_time(exec_time);
        #elif GET_DEADLINE /* CASE 1 */
            print_exec_time(exec_time);
            moment_timing_print(2); //moment_end
        #elif GET_OVERHEAD /* CASE 2 */
            //nothing
        #else /* CASE 3,4,5 and 6 */
            if(DELAY_EN && jump == 0 && ((delay_time = DEADLINE_TIME - exec_time - slice_time - dvfs_time - dvfs_table[cur_freq/100000-2][MIN_FREQ/100000-2] - dvfs_table[MIN_FREQ/100000-2][cur_freq/100000-2]) > 0)){
                start_timing();
				sleep_in_delay(delay_time, cur_freq);
                end_timing();
                actual_delay_time = exec_timing();
            }else
                delay_time = 0;
            moment_timing_print(2); //moment_end
            print_delay_time(delay_time, actual_delay_time);
            print_exec_time(exec_time);
            print_total_time(exec_time + slice_time + dvfs_time + actual_delay_time);
        #endif
        fclose_all();//TJSong

        // Write out predicted time & print out frequency used
        #if HETERO_EN
            print_predicted_time(predicted_exec_time.big);
            print_predicted_time(predicted_exec_time.little);
        #else
            #if CORE
                print_predicted_time(predicted_exec_time.big);
            #else
                print_predicted_time(predicted_exec_time.little);
            #endif
        #endif

    //---------------------modified by TJSong----------------------//

        }
        else
        {                           /* decryption in Cipher Block Chaining mode */
            printf("Slicing not implemented for decryption. Please implement.\n");
            exit(1);

            set_key(key, key_len, dec, ctx);
        
            err = decfile(fin, fout, ctx, argv[argv_i + 1], argv[argv_i + 2]);
        }
    exit:   
        if(fout) 
            fclose(fout);
        if(fin)
            fclose(fin);
    } // for argv_i

    return err;
}
