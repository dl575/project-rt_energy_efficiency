
int callme(char * maybe, int i) {
  return 0;
}

int main() {
  /* This is a comment */
  // This is another comment
  int a, b, c;
  int i;
  char* maybe;
  a = 0;
  b = 1;
  c = 2;
  // More comments
  /* Multi-line
     comment
     */
  for (i = 0; i < 10; i++) {
    if (a == 10 && b == c) {
      b = c;
    } else {
      b = a;
    }
    if (a < -1) {
      b = 10;
    }
    if (c >= 5 || a) {
      c = 4;
      callme(maybe, i);
    }
  }
  /* more multi-line
     */

  int while_var;
  while(while_var) {
    a = 1;
  }
  
  return 0;
}
