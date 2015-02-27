typedef int uint8_t
;
int main()
{
  int a;
  int *a_ptr;
  {
    // Inline function: func;
    int a_rename0 = a;
    // End of arguments;
    int c_rename0 = 5;
    int b_rename0 = c_rename0;
    if (a_rename0 || a_ptr)
    {
      a_rename0++;
    }

    return0:
    ;

  }
  int b[5] = {1, 2, 3, 4, 5};
  {
    // Inline function: array_func;
    // End of arguments;
    int i_rename1;
    for (i_rename1 = 0; i_rename1 < 5; i_rename1++)
    {
      if (b[i_rename1])
      {
        b[i_rename1] = 1337 + i_rename1;
      }

    }

    return1:
    ;

  }
  uint8_t board[4][4];
  {
    // Inline function: drawBoard;
    // End of arguments;
    uint8_t x_rename2;
    uint8_t y_rename2;
    for (x_rename2 = 0; x_rename2 < 4; x_rename2++)
    {
      for (y_rename2 = 0; y_rename2 < 4; y_rename2++)
      {
        if (board[x_rename2][y_rename2] != 0)
        {
          printf("a\n");
        }
        else
        {
          printf("b\n");
        }

      }

    }

    return2:
    ;

  }
}


