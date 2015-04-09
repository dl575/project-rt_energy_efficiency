typedef int FILE;
FILE *fopen();
int global;
float main_slice(int argc, char *argv[])
{
  int loop_counter[4] = {0, 0, 0, 0};
  {}
  {}
  {}
  {}
  {
    int func_result0;
    {
      int return_value;
      {}
      {
        return_value = 1;
        goto return0;
      }
      return0:
      ;

      func_result0 = return_value;
    }
    if (func_result0 == 1)
    {
      loop_counter[0]++;
      {}
    }

  }
  {
    int func_result0;
    {
      int return_value;
      {}
      {
        return_value = 1;
        goto return1;
      }
      return1:
      ;

      func_result0 = return_value;
    }
    if (func_result0)
    {
      loop_counter[1]++;
      {}
    }

  }
  {
    int func_result0;
    {
      int return_value;
      {}
      {
        return_value = 1;
        goto return2;
      }
      return2:
      ;

      func_result0 = return_value;
    }
    int func_result1;
    {
      int return_value;
      {}
      {
        return_value = 1;
        goto return3;
      }
      return3:
      ;

      func_result1 = return_value;
    }
    if (func_result0 || func_result1)
    {
      loop_counter[2]++;
      {}
    }

  }
  {
    FILE *fopen_result0;
    fopen_result0 = fopen(argv[1], "rb");
    if (!(fin = fopen_result0))
    {
      loop_counter[3]++;
      {}
    }

  }
  {
    print_loop_counter:
    ;

    {
      printf("loop counter = (");
      int i;
      for (i = 0; i < 4; i++)
        printf("%d, ", loop_counter[i]);

      printf(")\n");
    }
    print_loop_counter_end:
    ;

  }
  {
    predict_exec_time:
    ;

    float exec_time;
    exec_time = 0;
    return exec_time;
  }
}


