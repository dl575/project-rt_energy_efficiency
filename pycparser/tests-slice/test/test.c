//#include <stdio.h>

int sum (int one, int two) {
  int s = 0;
  s = one + two;
  return s;
}

void check() {
  int a = 1;
  int b = 2;
  if (a) return;
  int c = 1 + 2;
  return;
}

int main() {
  int a = 1;
  int b = 2;
  int *ptr;

  int c;
  sum(a, b);
  c = 0;

  if (c) {
    c = sum(a, c);
    printf("c = %d\n", c);
  } else {
    c = sum(b, c);
    printf("c = %d\n", c);
  }
  /*
  if (sum(a,c) + a) {
    a++;
  }
  */

  int i;
  for (i = 0; i < 10; i ++)
    a++;

  check();
  printf("c = %d\n", c);

  return 0;
}
