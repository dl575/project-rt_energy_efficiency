int main()
{
  int loop_counter[2] = {0, 0};
  int a[5] = {1, 2, 3, 4, 5};
  printf("a[0] = %d\n", a[0]);
  {
    int i_rename0;
    for (i_rename0 = 0; i_rename0 < 5; i_rename0++)
    {
      loop_counter[0]++;
      if (a[i_rename0])
      {
        loop_counter[1]++;
        a[i_rename0] = 1337 + i_rename0;
      }

    }

    return0:
    ;

  }
  printf("a[0] = %d\n", a[0]);
  printf("hello\n");
  return 0;
  {
    print_loop_counter:
    

    printf("loop counter = (");
    int i;
    for (i = 0; i < 2; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
}


