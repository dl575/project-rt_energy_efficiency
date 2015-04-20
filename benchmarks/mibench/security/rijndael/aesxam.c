
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

//---------------------modified by TJSong----------------------//
//set global variables
struct timeval start, end, moment;
int slice_time = 0;
int dvfs_time = 0;

//define benchmarks-depenent varaibles & constants
#if CORE //big
#define OVERHEAD_TIME 135874 //overhead deadline
#define AVG_OVERHEAD_TIME 48630 //avg overhead deadline
#define DEADLINE_TIME (int)((138638*SWEEP)/100) // max_exec * sweep / 100
#define MAX_DVFS_TIME 2779 //max dvfs time
#define AVG_DVFS_TIME 1135 //average dvfs time
#else //LITTLE
#define OVERHEAD_TIME 168542 //overhead deadline
#define AVG_OVERHEAD_TIME 56606 //avg overhead deadline
#define DEADLINE_TIME (int)((182846*SWEEP)/100) // max_exec * sweep / 100
#define MAX_DVFS_TIME 2581 //max dvfs time
#define AVG_DVFS_TIME 1146 //average dvfs time
#endif
//---------------------modified by TJSong----------------------//

void fillrand(char *buf, int len)
{   static unsigned long a[2], mt = 1, count = 4;
    static char          r[4];
    int                  i;

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

int encfile(FILE *fin, FILE *fout, aes *ctx, char* fn)
{   char            inbuf[16], outbuf[16];
    fpos_t          flen;
    unsigned long   i=0, l=0;

    fillrand(outbuf, 16);           /* set an IV for CBC mode           */
    fseek(fin, 0, SEEK_END);        /* get the length of the file       */
    fgetpos(fin, &flen);            /* and then reset to start          */
    fseek(fin, 0, SEEK_SET);        
    fwrite(outbuf, 1, 16, fout);    /* write the IV to the output       */
    fillrand(inbuf, 1);             /* make top 4 bits of a byte random */
    l = 15;                         /* and store the length of the last */
                                    /* block in the lower 4 bits        */
    //inbuf[0] = ((char)flen & 15) | (inbuf[0] & ~15);

    while(!feof(fin))               /* loop to encrypt the input file   */
    {                               /* input 1st 16 bytes to buf[1..16] */
        i = fread(inbuf + 16 - l, 1, l, fin);  /*  on 1st round byte[0] */
                                               /* is the length code    */
        if(i < l) break;            /* if end of the input file reached */

        for(i = 0; i < 16; ++i)         /* xor in previous cipher text  */
            inbuf[i] ^= outbuf[i]; 

        encrypt(inbuf, outbuf, ctx);    /* and do the encryption        */

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

        encrypt(inbuf, outbuf, ctx);    /* encrypt and output it        */

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

float slice(int argc, char *argv[])
{
  int loop_counter[46] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
{}
  FILE *fin = 0;
  FILE *fout = 0;
  char *cp;
  char ch;
{}
  int i = 0;
{}
  int key_len = 0;
  int err = 0;
  aes ctx[1];
  if ((argc != 5) || ((toupper(*argv[3]) != 'D') && (toupper(*argv[3]) != 'E')))
  {
    loop_counter[0]++;
{}
    err = -1;
  }

  cp = argv[4];
  i = 0;
  while ((i < 64) && (*cp))
  {
    loop_counter[1]++;
    ch = toupper(*(cp++));
    if ((ch >= '0') && (ch <= '9'))
    {
      loop_counter[2]++;
{}
    }
    else
      if ((ch >= 'A') && (ch <= 'F'))
    {
      loop_counter[3]++;
{}
    }
    else
    {
{}
      err = -2;
    }


    if ((i++) & 1)
    {
      loop_counter[4]++;
{}
    }

  }

  if (*cp)
  {
    loop_counter[5]++;
{}
    err = -3;
  }
  else
    if ((i < 32) || (i & 15))
  {
    loop_counter[6]++;
{}
    err = -4;
  }


  key_len = i / 2;
  if (!(fin = fopen(argv[1], "rb")))
  {
    loop_counter[7]++;
{}
    err = -5;
  }

  if (!(fout = fopen(argv[2], "wb")))
  {
    loop_counter[8]++;
{}
    err = -6;
  }

  if (toupper(*argv[3]) == 'E')
  {
    loop_counter[9]++;
    {
      aes_ret return_value;
      const enum aes_key f_rename0 = enc;
      const word n_bytes_rename0 = key_len;
{}
{}
{}
{}
      if ((((n_bytes_rename0 & 7) || (n_bytes_rename0 < 16)) || (n_bytes_rename0 > 32)) || ((!(f_rename0 & 1)) && (!(f_rename0 & 2))))
      {
        loop_counter[10]++;
        {
          return_value = n_bytes_rename0 ? ctx->mode &= ~0x03, 0 : (aes_ret) (ctx->Nkey << 2);
        }
      }

      ctx->mode = (ctx->mode & (~0x03)) | (((byte) f_rename0) & 0x03);
      ctx->Nkey = n_bytes_rename0 >> 2;
      ctx->Nrnd = (ctx->Nkey > Ncol ? ctx->Nkey : Ncol) + 6;
{}
{}
{}
{}
{}
{}
{}
      if ((ctx->mode & 3) != enc)
      {
        loop_counter[11]++;
        word i_rename0;
{}
{}
{}
{}
{}
{}
{}
        for (i_rename0 = 1; i_rename0 < ctx->Nrnd; ++i_rename0)
        {
          loop_counter[12]++;
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
      }

      {
        return_value = 1;
      }
    }
    {
      int return_value;
      char inbuf_rename1[16];
      char outbuf_rename1[16];
      fpos_t flen_rename1;
      unsigned long i_rename1 = 0;
      unsigned long l_rename1 = 0;
      {
        int len_rename4 = 16;
{}
        static unsigned long mt_rename4 = 1;
        static unsigned long count_rename4 = 4;
        static char r_rename4[4];
        int i_rename4;
        if (mt_rename4)
        {
          loop_counter[13]++;
          mt_rename4 = 0;
{}
{}
        }

        for (i_rename4 = 0; i_rename4 < len_rename4; ++i_rename4)
        {
          loop_counter[14]++;
          if (count_rename4 == 4)
          {
            loop_counter[15]++;
{}
            count_rename4 = 0;
          }

          outbuf_rename1[i_rename4] = r_rename4[count_rename4++];
        }

      }
{}
      fgetpos(fin, &flen_rename1);
{}
{}
      {
        int len_rename5 = 1;
{}
        static unsigned long mt_rename5 = 1;
        static unsigned long count_rename5 = 4;
        static char r_rename5[4];
        int i_rename5;
        if (mt_rename5)
        {
          loop_counter[16]++;
          mt_rename5 = 0;
{}
{}
        }

        for (i_rename5 = 0; i_rename5 < len_rename5; ++i_rename5)
        {
          loop_counter[17]++;
          if (count_rename5 == 4)
          {
            loop_counter[18]++;
{}
            count_rename5 = 0;
          }

          inbuf_rename1[i_rename5] = r_rename5[count_rename5++];
        }

      }
      l_rename1 = 15;
      //inbuf_rename1[0] = (((char) flen_rename1) & 15) | (inbuf_rename1[0] & (~15));
      while (!feof(fin))
      {
        loop_counter[19]++;
        i_rename1 = fread((inbuf_rename1 + 16) - l_rename1, 1, l_rename1, fin);
        if (i_rename1 < l_rename1)
        {
          loop_counter[20]++;
        }

        for (i_rename1 = 0; i_rename1 < 16; ++i_rename1)
        {
          loop_counter[21]++;
          inbuf_rename1[i_rename1] ^= outbuf_rename1[i_rename1];
        }

        {
          aes_ret return_value;
{}
{}
{}
{}
{}
          if (!(ctx->mode & 0x01))
          {
            loop_counter[22]++;
            return_value = 0;
          }

{}
{}
{}
{}
{}
{}
{}
{}
{}
          {
            return_value = 1;
          }
        }
        if (fwrite(outbuf_rename1, 1, 16, fout) != 16)
        {
          loop_counter[23]++;
{}
          {
            return_value = -7;
          }
        }

        l_rename1 = 16;
      }

      if (l_rename1 == 15)
      {
        loop_counter[24]++;
        ++i_rename1;
      }

      if (i_rename1)
      {
        loop_counter[25]++;
        while (i_rename1 < 16)
        {
          loop_counter[26]++;
          inbuf_rename1[i_rename1++] = 0;
        }

        for (i_rename1 = 0; i_rename1 < 16; ++i_rename1)
        {
          loop_counter[27]++;
          inbuf_rename1[i_rename1] ^= outbuf_rename1[i_rename1];
        }

        {
          aes_ret return_value;
{}
{}
{}
{}
{}
          if (!(ctx->mode & 0x01))
          {
            loop_counter[28]++;
            return_value = 0;
          }

{}
{}
{}
{}
{}
{}
{}
{}
{}
          {
            return_value = 1;
          }
        }
        if (fwrite(outbuf_rename1, 1, 16, fout) != 16)
        {
          loop_counter[29]++;
{}
          {
            return_value = -8;
          }
        }

      }

      {
        return_value = 0;
      }
      err = return_value;
    }
  }
  else
  {
    {
      aes_ret return_value;
      const enum aes_key f_rename2 = dec;
      const word n_bytes_rename2 = key_len;
{}
{}
{}
{}
      if ((((n_bytes_rename2 & 7) || (n_bytes_rename2 < 16)) || (n_bytes_rename2 > 32)) || ((!(f_rename2 & 1)) && (!(f_rename2 & 2))))
      {
        loop_counter[30]++;
        {
          return_value = n_bytes_rename2 ? ctx->mode &= ~0x03, 0 : (aes_ret) (ctx->Nkey << 2);
        }
      }

      ctx->mode = (ctx->mode & (~0x03)) | (((byte) f_rename2) & 0x03);
      ctx->Nkey = n_bytes_rename2 >> 2;
      ctx->Nrnd = (ctx->Nkey > Ncol ? ctx->Nkey : Ncol) + 6;
{}
{}
{}
{}
{}
{}
{}
      if ((ctx->mode & 3) != enc)
      {
        loop_counter[31]++;
        word i_rename2;
{}
{}
{}
{}
{}
{}
{}
        for (i_rename2 = 1; i_rename2 < ctx->Nrnd; ++i_rename2)
        {
          loop_counter[32]++;
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
      }

      {
        return_value = 1;
      }
    }
    {
      int return_value;
      char inbuf1_rename3[16];
      char inbuf2_rename3[16];
      char outbuf_rename3[16];
      char *bp1_rename3;
      char *bp2_rename3;
      char *tp_rename3;
      int i_rename3;
      int l_rename3;
      int flen_rename3;
      if (fread(inbuf1_rename3, 1, 16, fin) != 16)
      {
        loop_counter[33]++;
{}
        {
          return_value = 9;
        }
      }

      i_rename3 = fread(inbuf2_rename3, 1, 16, fin);
      if (i_rename3 && (i_rename3 != 16))
      {
        loop_counter[34]++;
{}
        {
          return_value = -10;
        }
      }

      {
        aes_ret return_value;
{}
{}
{}
{}
{}
        if (!(ctx->mode & 0x02))
        {
          loop_counter[35]++;
          return_value = 0;
        }

{}
{}
{}
{}
{}
{}
{}
{}
{}
        {
          return_value = 1;
        }
      }
      for (i_rename3 = 0; i_rename3 < 16; ++i_rename3)
      {
        loop_counter[36]++;
        outbuf_rename3[i_rename3] ^= inbuf1_rename3[i_rename3];
      }

      flen_rename3 = outbuf_rename3[0] & 15;
      l_rename3 = 15;
      bp1_rename3 = inbuf1_rename3;
      bp2_rename3 = inbuf2_rename3;
      while (1)
      {
        loop_counter[37]++;
        i_rename3 = fread(bp1_rename3, 1, 16, fin);
        if (i_rename3 != 16)
        {
          loop_counter[38]++;
        }

        if (fwrite((outbuf_rename3 + 16) - l_rename3, 1, l_rename3, fout) != ((unsigned long) l_rename3))
        {
          loop_counter[39]++;
{}
          {
            return_value = -11;
          }
        }

        {
          aes_ret return_value;
{}
{}
{}
{}
{}
          if (!(ctx->mode & 0x02))
          {
            loop_counter[40]++;
            return_value = 0;
          }

{}
{}
{}
{}
{}
{}
{}
{}
{}
          {
            return_value = 1;
          }
        }
        for (i_rename3 = 0; i_rename3 < 16; ++i_rename3)
        {
          loop_counter[41]++;
          outbuf_rename3[i_rename3] ^= bp2_rename3[i_rename3];
        }

        l_rename3 = i_rename3;
        tp_rename3 = bp1_rename3, bp1_rename3 = bp2_rename3, bp2_rename3 = tp_rename3;
      }

      l_rename3 = l_rename3 == 15 ? 1 : 0;
      flen_rename3 += 1 - l_rename3;
      if (flen_rename3)
      {
        loop_counter[42]++;
        if (fwrite(outbuf_rename3 + l_rename3, 1, flen_rename3, fout) != ((unsigned long) flen_rename3))
        {
          loop_counter[43]++;
{}
          {
            return_value = -12;
          }
        }

      }

      {
        return_value = 0;
      }
      err = return_value;
    }
  }

  exit:
  if (fout)
  {
    loop_counter[44]++;
{}
  }


  if (fin)
  {
    loop_counter[45]++;
{}
  }

{}
{}
  {
    //return_value = err;
    goto print_loop_counter;
  }
  print_loop_counter:
  {
{}
#if GET_PREDICT || DEBUG_EN
    int i;
    printf("loop counter = (");
    for (i = 0; i < 46; i++)
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
exec_time = 3.603160*loop_counter[1] + 11.057600*loop_counter[3] + 1.122790*loop_counter[19] + 0.000000;
#else //LITTLE
exec_time = 4.947220*loop_counter[1] + 19.778900*loop_counter[3] + 2.126860*loop_counter[19] + 0.000000;
#endif
    return exec_time;
  }

}


int main(int argc, char *argv[])
{   
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
    #if !GET_PREDICT && !GET_DEADLINE && !GET_OVERHEAD && !PREDICT_EN /* CASE 3 */
        //slice_time=0; dvfs_time=0;
        moment_timing_print(0); //moment_start
    #endif
    #if !GET_PREDICT && !GET_DEADLINE && !GET_OVERHEAD && PREDICT_EN /* CASE 4 */
        moment_timing_print(0); //moment_start
        
        start_timing();
        predicted_exec_time = slice(argc, argv); //slice
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
 
    FILE    *fin = 0, *fout = 0;
    char    *cp, ch, key[32];
    int     i=0, by=0, key_len=0, err = 0;
    aes     ctx[1];

    if(argc != 5 || (toupper(*argv[3]) != 'D' && toupper(*argv[3]) != 'E'))
    {
        printf("usage: rijndael in_filename out_filename [d/e] key_in_hex\n"); 
        err = -1; goto exit;
    }

    cp = argv[4];   /* this is a pointer to the hexadecimal key digits  */
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

    if(!(fin = fopen(argv[1], "rb")))   /* try to open the input file */
    {
        printf("The input file: %s could not be opened\n", argv[1]); 
        err = -5; goto exit;
    }

    if(!(fout = fopen(argv[2], "wb")))  /* try to open the output file */
    {
        printf("The output file: %s could not be opened\n", argv[1]); 
        err = -6; goto exit;
    }

    if(toupper(*argv[3]) == 'E')
    {                           /* encryption in Cipher Block Chaining mode */
        set_key(key, key_len, enc, ctx);

        err = encfile(fin, fout, ctx, argv[1]);
    }
    else
    {                           /* decryption in Cipher Block Chaining mode */
        set_key(key, key_len, dec, ctx);
    
        err = decfile(fin, fout, ctx, argv[1], argv[2]);
    }
exit:   
    if(fout) 
        fclose(fout);
    if(fin)
        fclose(fin);


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
    #if !GET_PREDICT && !GET_DEADLINE && !GET_OVERHEAD /* CASE 3 and 4 */
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
    //#if DEBUG_EN
        print_predicted_time(predicted_exec_time);
        print_freq(); 
    //#endif
//---------------------modified by TJSong----------------------//
    return err;
}
