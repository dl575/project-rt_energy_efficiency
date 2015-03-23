float main_slice()
{
  int loop_counter[3] = {0, 0, 0};
  int c;
  {}
  switch (c)
  {
    case 1:
      loop_counter[0]++;
      {}
      break;

    case 2:
      loop_counter[1]++;
      {}
      break;

    default:
      loop_counter[2]++;
      {}

  }

  {
    print_loop_counter:
    ;

    {
      printf("loop counter = (");
      int i;
      for (i = 0; i < 3; i++)
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


