int main()
{
  int a;
  int b;
  int c;
  int d;
  {
    int func_result0;
    {
      // Inline function: func;
      int return_value;
      int a_rename0 = 1;
      // End of arguments;
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
      a++;
    }

  }
  {
    int func_result0;
    {
      // Inline function: func;
      int return_value;
      int a_rename1 = a;
      // End of arguments;
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
      b++;
    }

  }
  {
    int func_result0;
    {
      // Inline function: func;
      int return_value;
      int a_rename2 = c;
      // End of arguments;
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
      // Inline function: func;
      int return_value;
      int a_rename3 = d;
      // End of arguments;
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
      c++;
    }

  }
}


