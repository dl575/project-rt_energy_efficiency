typedef int bool
;
int global_big_array[4][4][4]
;
int global_array[4]
;
int global_var3
;
int global_var2
;
int global_var1
;
int global_var0
;
struct thing global_thing_s
;
typedef int custom_type
;
enum custom_enum {one = 1, two = 2, three = 3}
;
int declared_global
;
struct struct_t
{
  int a;
}
;
int func(int arg1, int arg2[4], int arg3[3][4][5], struct struct_t *arg_struct)
{
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
  declared_global++;
  arg3[1][2][3]++;
  arg_struct->a = 1;
  enum custom_enum e;
  e = one;
  a = global_var0 + global_var1;
  global_var2 = b;
  thing_s->first = 1;
  thing_s->second = 2;
  global_thing_s->first = 3;
  array[0] = 1;
  array[1] = global_array[a];
  global_big_array[1][2][3]++;
  {
    // Inline function: increment_global;
    // End of arguments;
    global_var3++;
    printf("huzzah\n");
    return0:
    ;

  }
  {
    // Inline function: increment;
    int return_value;
    int val_rename1 = b;
    // End of arguments;
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
    // Inline function: increment_array;
    // End of arguments;
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


