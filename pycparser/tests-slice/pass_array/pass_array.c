//#include <stdio.h>

void func(int array[5]) {
  int i;
  for (i = 0; i < 5; i++) {
    if (array[i]) {
      array[i] = 1337 + i;
    }
  }
}

int main() {
  int a[5] = {1,2,3,4,5};
  printf("a[0] = %d\n", a[0]);
  func(a);
  printf("a[0] = %d\n", a[0]);
  printf("hello\n");
  return 0;
}
