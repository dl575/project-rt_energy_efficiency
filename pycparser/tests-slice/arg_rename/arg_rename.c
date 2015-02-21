void func(int a, int *pointer) {
  // Declarations that should be renamed
  int c = 5;
  int b = c;
  if (a || pointer) {
    a++;
  }
}

int main() {
  int a;
  int *a_ptr;
  func(a, a_ptr);
}
