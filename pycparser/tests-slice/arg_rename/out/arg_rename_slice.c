typedef int uint8_t;
void main_slice()
{
  int loop_counter[1] = {0};
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
  {
    print_loop_counter:
    printf("loop counter = (");

    int i;
    for (i = 0; i < 1; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
}


