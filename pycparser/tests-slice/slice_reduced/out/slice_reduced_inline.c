typedef int fpos_t
;
typedef int aes
;
typedef int FILE
;
int main(int argc, char *argv[])
{
  {
    // Inline function: encfile;
    int return_value;
    // End of arguments;
    char outbuf_rename0[16];
    {
      // Inline function: fillrand;
      int len_rename1 = 16;
      // End of arguments;
      static unsigned long count_rename1 = 4;
      static char r_rename1[4];
      int i_rename1;
      for (i_rename1 = 0; i_rename1 < len_rename1; ++i_rename1)
      {
        if (count_rename1 == 4)
        {
          count_rename1 = 0;
        }

        outbuf_rename0[i_rename1] = r_rename1[count_rename1++];
      }

      return1:
      ;

    }
    {
      return_value = 0;
      goto return0;
    }
    return0:
    ;

    err = return_value;
  }
  return err;
}


