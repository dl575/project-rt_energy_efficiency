/* NIST Secure Hash Algorithm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sha.h"
#include "timing.h"

void slice(int argc, char **argv)
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
    int i;
    printf("loop counter = (");
    for (i = 0; i < 47; i++)
      printf("%d, ", loop_counter[i]++);
    printf(")\n");

{}
float exec_time;
exec_time = 2.058300*loop_counter[23] + 1988.040000*loop_counter[25] + -14.939600*loop_counter[27] + -504.579000*loop_counter[34] + 0.000000;
printf("predicted time = %f\n", exec_time);


  }

}

int main(int argc, char **argv)
{
    FILE *fin;
    SHA_INFO sha_info;

    slice(argc, argv);
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
    print_timing();

    return(0);
}
