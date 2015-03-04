int main()
{
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
      a[0]++;
    {
      // Inline function: copy;
      int return_value;
      int a_rename0 = a[0];
      // End of arguments;
      {
        return_value = a_rename0;
        goto return0;
      }
      return0:
      ;

      a[0] = return_value;
    }
    {
      // Inline function: copy;
      int return_value;
      int a_rename1 = a[1];
      // End of arguments;
      {
        return_value = a_rename1;
        goto return1;
      }
      return1:
      ;

      a[1] = return_value;
    }
    {
      // Inline function: copy;
      int return_value;
      int a_rename2 = a[1];
      // End of arguments;
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

}


