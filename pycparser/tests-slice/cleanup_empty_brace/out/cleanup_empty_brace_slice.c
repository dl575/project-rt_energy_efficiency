void main_slice()
{
  int loop_counter[4] = {0, 0, 0, 0};
  int a;
  if (a = 1)
  {
  }

  if (a + 1)
  {
    loop_counter[1]++;
  }

  switch (a)
  {
    case 1:
      break;

    default:
      break;

  }

  {
    print_loop_counter:
    printf("loop counter = (");

    int i;
    for (i = 0; i < 4; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
}


