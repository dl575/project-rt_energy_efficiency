int global;

int func(int a) {
  return 1;
}

int main(int argc, char *argv[]) {
  int a, b, c, d;

  if (func(1) == 1) {
    a++;
  }
  if (func(a)) {
    b++;
  }

  if (func(c) || func(d)) {
    c++;
  }

  if (!(fin = fopen(argv[1], "rb"))) {
    d++;
  }
}
