int func(int a) {
  return 1;
}

int main() {
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
}
