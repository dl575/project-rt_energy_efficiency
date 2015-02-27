typedef int uint8_t
;
int main()
{
  uint8_t board[4][4];
  {
    // Inline function: drawBoard;
    // End of arguments;
    uint8_t x_rename0;
    uint8_t y_rename0;
    for (x_rename0 = 0; x_rename0 < 4; x_rename0++)
    {
      for (y_rename0 = 0; y_rename0 < 4; y_rename0++)
      {
        if (board[x_rename0][y_rename0] != 0)
        {
          printf("a\n");
        }
        else
        {
          printf("b\n");
        }

      }

    }

    return0:
    ;

  }
}


