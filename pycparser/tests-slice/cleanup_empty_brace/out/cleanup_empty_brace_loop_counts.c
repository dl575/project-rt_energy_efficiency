int main()
{
  int loop_counter[4] = {0, 0, 0, 0};
  int a;
  if (a = 1)
    loop_counter[0]++;

  if (a + 1)
  {
    loop_counter[1]++;
  }

  switch (a)
  {
    case 1:
      loop_counter[2]++;
    {
    }
    {
    }
    {
    }
    {
    }
      break;

    default:
      loop_counter[3]++;
      break;

  }

  {
    {
      {
        {
        }
      }
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


