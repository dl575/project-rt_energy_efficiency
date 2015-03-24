typedef int bool;
int global_big_array[4][4][4];
int global_array[4];
int global_var3;
int global_var2;
int global_var1;
int global_var0;
struct thing global_thing_s;
typedef int custom_type;
enum custom_enum {one = 1, two = 2, three = 3};
int declared_global;
struct struct_t
{
  int a;
};
int func(int arg1, int arg2[4], int arg3[3][4][5], struct struct_t *arg_struct)
{
  int global_array_rename[4];
  int global_array_i0;
  for (global_array_i0 = 0; global_array_i0 < 4; global_array_i0++)
  {
    global_array_rename[global_array_i0] = global_array[global_array_i0];
  }

  int global_var1_rename = global_var1;
  int global_var0_rename = global_var0;
  int global_var3_rename = global_var3;
  int global_var2_rename = global_var2;
  struct thing global_thing_s_rename = global_thing_s;
  int declared_global_rename = declared_global;
  int global_big_array_rename[4][4][4];
  int global_big_array_i0;
  for (global_big_array_i0 = 0; global_big_array_i0 < 4; global_big_array_i0++)
  {
    int global_big_array_i1;
    for (global_big_array_i1 = 0; global_big_array_i1 < 4; global_big_array_i1++)
    {
      int global_big_array_i2;
      for (global_big_array_i2 = 0; global_big_array_i2 < 4; global_big_array_i2++)
      {
        global_big_array_rename[global_big_array_i0][global_big_array_i1][global_big_array_i2] = global_big_array[global_big_array_i0][global_big_array_i1][global_big_array_i2];
      }

    }

  }

  struct struct_t *arg_struct_rename = arg_struct;
  int arg3_rename[3][4][5];
  int arg3_i0;
  for (arg3_i0 = 0; arg3_i0 < 3; arg3_i0++)
  {
    int arg3_i1;
    for (arg3_i1 = 0; arg3_i1 < 4; arg3_i1++)
    {
      int arg3_i2;
      for (arg3_i2 = 0; arg3_i2 < 5; arg3_i2++)
      {
        arg3_rename[arg3_i0][arg3_i1][arg3_i2] = arg3[arg3_i0][arg3_i1][arg3_i2];
      }

    }

  }

  int arg2_rename[4];
  int arg2_i0;
  for (arg2_i0 = 0; arg2_i0 < 4; arg2_i0++)
  {
    arg2_rename[arg2_i0] = arg2[arg2_i0];
  }

  int a;
  int b = a;
  custom_type c;
  struct thing
  {
    int first;
    int second;
  };
  struct thing thing_s;
  int array[4];
  int d = (custom_type) c;
  declared_global_rename++;
  arg3_rename[1][2][3]++;
  arg_struct_rename->a = 1;
  enum custom_enum e;
  e = one;
  a = global_var0_rename + global_var1_rename;
  global_var2_rename = b;
  thing_s->first = 1;
  thing_s->second = 2;
  global_thing_s_rename->first = 3;
  array[0] = 1;
  array[1] = global_array_rename[a];
  global_big_array_rename[1][2][3]++;
  {
    global_var3_rename++;
    printf("huzzah\n");
    return0:
    ;

  }
  {
    int return_value;
    int val_rename1 = b;
    val_rename1++;
    {
      return_value = val_rename1;
      goto return1;
    }
    return1:
    ;

    b = return_value;
  }
  {
    int i_rename2;
    for (i_rename2 = 0; i_rename2 < 4; i_rename2++)
    {
      array[i_rename2]++;
    }

    return2:
    ;

  }
  bool a_bool;
  a_bool = true;
}


