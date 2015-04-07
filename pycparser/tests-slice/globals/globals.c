int increment(int val) {
  val++;
  return val;
}

void increment_global() {
  global_var3++;
  printf("huzzah\n");
}

void increment_array(int array[4]) {
  int i;
  for (i = 0; i < 4; i++) {
    array[i]++;
  }
}

typedef int custom_type;
enum custom_enum {
  one = 1,
  two = 2,
  three = 3
};

int declared_global;

struct struct_t {
  int a;
};

typedef struct struct_t struct2_t;

int func(int arg1, int arg2[4], int arg3[3][4][5], struct struct_t *arg_struct, int *arg_int_ptr, struct2_t *arg_struct2) {
  int a;  
  int b = a;
  custom_type c;
  struct thing {
    int first;
    int second;
  };
  struct thing thing_s;
  int array[4];
  int d = (custom_type) c;
  declared_global++;

  arg3[1][2][3]++;
  arg_struct->a = 1;
  arg_struct2->a = 1;
  (*arg_int_ptr)++;

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

  increment_global();
  b = increment(b);

  increment_array(array);

  bool a_bool;
  a_bool = true;
}
