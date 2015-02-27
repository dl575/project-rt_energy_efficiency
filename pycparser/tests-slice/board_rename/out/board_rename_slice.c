typedef int uint8_t;
void main_slice()
{
  int loop_counter[3] = {0, 0, 0};
  uint8_t board[4][4];
  {
    uint8_t x_rename0;
    uint8_t y_rename0;
    for (x_rename0 = 0; x_rename0 < 4; x_rename0++)
    {
      loop_counter[0]++;
      for (y_rename0 = 0; y_rename0 < 4; y_rename0++)
      {
        loop_counter[1]++;
        if (board[x_rename0][y_rename0] != 0)
        {
          loop_counter[2]++;
{}
        }
        else
        {
{}
        }

      }

    }

    return0:
    ;

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


