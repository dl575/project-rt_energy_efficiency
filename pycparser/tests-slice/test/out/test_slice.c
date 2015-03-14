void main_slice()
{
  int loop_counter[3] = {0, 0, 0};
  int a = 1;
  int b = 2;
  {}
  int c;
  {
    int return_value;
    int two_rename0 = b;
    int one_rename0 = a;
    int s_rename0 = 0;
    s_rename0 = one_rename0 + two_rename0;
    {
      return_value = s_rename0;
      goto return0;
    }
    return0:
    ;

  }
  c = 0;
  if (c)
  {
    loop_counter[0]++;
    {
      int return_value;
      int two_rename1 = c;
      int one_rename1 = a;
      int s_rename1 = 0;
      s_rename1 = one_rename1 + two_rename1;
      {
        return_value = s_rename1;
        goto return1;
      }
      return1:
      ;

      c = return_value;
    }
    {}
  }
  else
  {
    {
      int return_value;
      int two_rename2 = c;
      int one_rename2 = b;
      int s_rename2 = 0;
      s_rename2 = one_rename2 + two_rename2;
      {
        return_value = s_rename2;
        goto return2;
      }
      return2:
      ;

      c = return_value;
    }
    {}
  }

  int i;
  for (i = 0; i < 10; i++)
  {
    loop_counter[1]++;
    a++;
  }

  {
    int a_rename3 = 1;
    {}
    if (a_rename3)
    {
      loop_counter[2]++;
      goto return3;
    }

    {}
    {
      goto return3;
    }
    return3:
    ;

  }
  {}
  {
    goto print_loop_counter;
  }
  {
    print_loop_counter:
    printf("loop counter = (");

    int i;
    for (i = 0; i < 3; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
}


