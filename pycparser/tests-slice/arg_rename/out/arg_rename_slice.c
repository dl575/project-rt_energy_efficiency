typedef int uint8_t;
void main_slice()
{
  int loop_counter[6] = {0, 0, 0, 0, 0, 0};
  int a;
  int *a_ptr;
  {
    int a_rename0 = a;
    {}
    {}
    if (a_rename0 || a_ptr)
    {
      loop_counter[0]++;
      a_rename0++;
    }

    return0:
    ;

  }
  int b[5] = {1, 2, 3, 4, 5};
  {
    int i_rename1;
    for (i_rename1 = 0; i_rename1 < 5; i_rename1++)
    {
      loop_counter[1]++;
      if (b[i_rename1])
      {
        loop_counter[2]++;
        b[i_rename1] = 1337 + i_rename1;
      }

    }

    return1:
    ;

  }
  uint8_t board[4][4];
  {
    uint8_t x_rename2;
    uint8_t y_rename2;
    for (x_rename2 = 0; x_rename2 < 4; x_rename2++)
    {
      loop_counter[3]++;
      for (y_rename2 = 0; y_rename2 < 4; y_rename2++)
      {
        loop_counter[4]++;
        if (board[x_rename2][y_rename2] != 0)
        {
          loop_counter[5]++;
          {}
        }
        else
        {
          {}
        }

      }

    }

    return2:
    ;

  }
  {
    print_loop_counter:
    printf("loop counter = (");

    int i;
    for (i = 0; i < 6; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
}


