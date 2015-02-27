void func(int a, int *pointer) {
  // Declarations that should be renamed
  int c = 5;
  int b = c;
  if (a || pointer) {
    a++;
  }
}

void array_func(int array[5]) {
  int i;
  for (i = 0; i < 5; i++) {
    if (array[i]) {
      array[i] = 1337 + i;
    }
  }
}

#define SIZE 4
void drawBoard(uint8_t board[SIZE][SIZE]) {
  uint8_t x, y;
  for (x = 0; x < SIZE; x++) {
    for (y = 0; y < SIZE; y++) {
      if (board[x][y] != 0) {
        printf("a\n");
      } else {
        printf("b\n");
      }
    }
  }
}

int main() {
  int a;
  int *a_ptr;
  // Pass variable
  // Inlining should create renamed version and copy value
  func(a, a_ptr);

  // Pass array
  // Inlining should rename function variables with array name (no copy)
  int b[5] = {1,2,3,4,5};
  array_func(b);

  uint8_t board[SIZE][SIZE];
  drawBoard(board);
}
