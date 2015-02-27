int main()
{
  int loop_counter[1] = {0};
  int a[] = {1, 1};
  a[0]++;
  a[1]++;
  a[0] += 1;
  a[1] += 1;
  a[0] = a[0] + 2;
  a[1] = a[1] + 2;
  switch (a[0])
  {
    case 0:
      loop_counter[0]++;
      a[0]++;
    {
      int return_value;
      int a_rename0 = a[0];
      {
        return_value = a_rename0;
        goto return0;
      }
      return0:
      ;

      a[0] = return_value;
    }
    {
      int return_value;
      int a_rename1 = a[1];
      {
        return_value = a_rename1;
        goto return1;
      }
      return1:
      ;

      a[1] = return_value;
    }
    {
      int return_value;
      int a_rename2 = a[1];
      {
        return_value = a_rename2;
        goto return2;
      }
      return2:
      ;

    }
      break;

    default:
      break;

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


