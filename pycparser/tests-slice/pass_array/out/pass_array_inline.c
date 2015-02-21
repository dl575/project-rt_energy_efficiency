int main()
{
  int a[5] = {1, 2, 3, 4, 5};
  printf("a[0] = %d\n", a[0]);
  {
    // Inline function: func;
    // End of arguments;
    int i_rename0;
    for (i_rename0 = 0; i_rename0 < 5; i_rename0++)
    {
      if (a[i_rename0])
      {
        a[i_rename0] = 1337 + i_rename0;
      }

    }

    return0:
    ;

  }
  printf("a[0] = %d\n", a[0]);
  printf("hello\n");
  return 0;
}


