typedef int fpos_t;
typedef int aes;
typedef int FILE;
float main_slice(int argc, char *argv[])
{
  int loop_counter[2] = {0, 0};
  {
    {
    }
    char outbuf_rename0[16];
    {
      int len_rename1 = 16;
      static unsigned long count_rename1 = 4;
      static char r_rename1[4];
      int i_rename1;
      for (i_rename1 = 0; i_rename1 < len_rename1; ++i_rename1)
      {
        {}
        if (count_rename1 == 4)
        {
          loop_counter[1]++;
          count_rename1 = 0;
        }

        outbuf_rename0[i_rename1] = r_rename1[count_rename1++];
      }

      return1:
      ;

    }
    {
      {
      }
      goto return0;
    }
    return0:
    ;

    {
    }
  }
  {
    goto print_loop_counter;
  }
  {
    print_loop_counter:
    printf("loop counter = (");

    int i;
    for (i = 0; i < 2; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
  {
    predict_exec_time:
    ;

    float exec_time;
    exec_time = 0;
    return exec_time;
  }
}


