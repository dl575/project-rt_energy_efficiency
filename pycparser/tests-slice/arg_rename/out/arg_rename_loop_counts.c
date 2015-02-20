typedef int uint8_t;
int main()
{
  int loop_counter[1] = {0};
  int a;
  {
    int a_rename0 = a;
    int b_rename0;
    if (a_rename0)
    {
      loop_counter[0]++;
      a_rename0++;
    }

    return0:
    ;

  }
  {
    print_loop_counter:
    

    printf("loop counter = (");
    int i;
    for (i = 0; i < 1; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
}


