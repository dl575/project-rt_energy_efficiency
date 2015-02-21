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
  uint8_t board[SIZE][SIZE];
  drawBoard(board);
}
