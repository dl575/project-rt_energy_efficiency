/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#include "timing.h"

#define SIZE 24
uint32_t score=0;
uint8_t scheme=0;

void getColor(uint8_t value, char *color, size_t length) {
    uint8_t original[] = {8,255,1,255,2,255,3,255,4,255,5,255,6,255,7,255,9,0,10,0,11,0,12,0,13,0,14,0,255,0,255,0};
    uint8_t blackwhite[] = {232,255,234,255,236,255,238,255,240,255,242,255,244,255,246,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0};
    uint8_t bluered[] = {235,255,63,255,57,255,93,255,129,255,165,255,201,255,200,255,199,255,198,255,197,255,196,255,196,255,196,255,196,255,196,255};
    uint8_t *schemes[] = {original,blackwhite,bluered};
    uint8_t *background = schemes[scheme]+0;
    uint8_t *foreground = schemes[scheme]+1;
    if (value > 0) while (value--) {
        if (background+2<schemes[scheme]+sizeof(original)) {
            background+=2;
            foreground+=2;
        }
    }
    snprintf(color,length,"\e[38;5;%d;48;5;%dm",*foreground,*background);
}

void drawBoard(uint8_t board[SIZE][SIZE], int new_s) {
    uint8_t x,y;
    char c;
    char color[40], reset[] = "\e[m";
    printf("\e[H");

    printf("2048.c %17d pts\n\n",score);

    for (y=0;y<new_s;y++) {
        for (x=0;x<new_s;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            printf("       ");
            printf("%s",reset);
        }
        printf("\n");
        for (x=0;x<new_s;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            if (board[x][y]!=0) {
                char s[8];
                snprintf(s,8,"%u",(uint32_t)1<<board[x][y]);
                uint8_t t = 7-strlen(s);
                printf("%*s%s%*s",t-t/2,"",s,t/2,"");
            } else {
                printf("   ·   ");
            }
            printf("%s",reset);
        }
        printf("\n");
        for (x=0;x<new_s;x++) {
            getColor(board[x][y],color,40);
            printf("%s",color);
            printf("       ");
            printf("%s",reset);
        }
        printf("\n");
    }
    printf("\n");
    printf("        ←,↑,→,↓ or q        \n");
    printf("\e[A"); // one line up
}

uint8_t findTarget(uint8_t array[SIZE],uint8_t x,uint8_t stop) {
    uint8_t t;
    // if the position is already on the first, don't evaluate
    if (x==0) {
        return x;
    }
    for(t=x-1;t>=0;t--) {
        if (array[t]!=0) {
            if (array[t]!=array[x]) {
                // merge is not possible, take next position
                return t+1;
            }
            return t;
        } else {
            // we should not slide further, return this one
            if (t==stop) {
                return t;
            }
        }
    }
    // we did not find a
    return x;
}

bool slideArray(uint8_t array[SIZE], int new_s) {
    bool success = false;
    uint8_t x,t,stop=0;

    for (x=0;x<new_s;x++) {
        if (array[x]!=0) {
            t = findTarget(array,x,stop);
            // if target is not original position, then move or merge
            if (t!=x) {
                // if target is zero, this is a move
                if (array[t]==0) {
                    array[t]=array[x];
                } else if (array[t]==array[x]) {
                    // merge (increase power of two)
                    array[t]++;
                    // increase score
                    score+=(uint32_t)1<<array[t];
                    // set stop to avoid double merge
                    stop = t+1;
                }
                array[x]=0;
                success = true;
            }
        }
    }
    return success;
}

void rotateBoard(uint8_t board[SIZE][SIZE], int new_s) {
    uint8_t i,j,n=new_s;
    uint8_t tmp;
    for (i=0; i<n/2; i++) {
        for (j=i; j<n-i-1; j++) {
            tmp = board[i][j];
            board[i][j] = board[j][n-i-1];
            board[j][n-i-1] = board[n-i-1][n-j-1];
            board[n-i-1][n-j-1] = board[n-j-1][i];
            board[n-j-1][i] = tmp;
        }
    }
}

bool moveUp(uint8_t board[SIZE][SIZE], int new_s) {
    bool success = false;
    uint8_t x;
    for (x=0;x<new_s;x++) {
        success |= slideArray(board[x], new_s);
    }
    return success;
}

bool moveLeft(uint8_t board[SIZE][SIZE], int new_s) {
    bool success;
    rotateBoard(board, new_s);
    success = moveUp(board, new_s);
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    return success;
}

bool moveDown(uint8_t board[SIZE][SIZE], int new_s) {
    bool success;
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    success = moveUp(board, new_s);
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    return success;
}

bool moveRight(uint8_t board[SIZE][SIZE], int new_s) {
    bool success;
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    success = moveUp(board, new_s);
    rotateBoard(board, new_s);
    return success;
}

bool findPairDown(uint8_t board[SIZE][SIZE], int new_s) {
    bool success = false;
    uint8_t x,y;
    for (x=0;x<new_s;x++) {
        for (y=0;y<new_s-1;y++) {
            if (board[x][y]==board[x][y+1]) return true;
        }
    }
    return success;
}

uint8_t countEmpty(uint8_t board[SIZE][SIZE], int new_s) {
    uint8_t x,y;
    uint8_t count=0;
    for (x=0;x<new_s;x++) {
        for (y=0;y<new_s;y++) {
            if (board[x][y]==0) {
                count++;
            }
        }
    }
    return count;
}

bool gameEnded(uint8_t board[SIZE][SIZE], int new_s) {
    bool ended = true;
    if (countEmpty(board, new_s)>0) return false;
    if (findPairDown(board, new_s)) return false;
    rotateBoard(board, new_s);
    if (findPairDown(board, new_s)) ended = false;
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    rotateBoard(board, new_s);
    ended = false;
    return ended;
}

void addRandom(uint8_t board[SIZE][SIZE], int new_s) {
    static bool initialized = false;
    uint8_t x,y;
    uint8_t r,len=0;
    uint8_t n,list[SIZE*SIZE][2];

    if (!initialized) {
        //srand(time(NULL));
    srand(0);
        initialized = true;
    }

    for (x=0;x<new_s;x++) {
        for (y=0;y<new_s;y++) {
            if (board[x][y]==0) {
                list[len][0]=x;
                list[len][1]=y;
                len++;
            }
        }
    }

    if (len>0) {
        r = rand()%len;
        x = list[r][0];
        y = list[r][1];
        n = (rand()%10)/9+1;
        board[x][y]=n;
    }
}

void initBoard(uint8_t board[SIZE][SIZE], int new_s) {
    uint8_t x,y;
    for (x=0;x<SIZE;x++) {
        for (y=0;y<SIZE;y++) {
            board[x][y]=0;
        }
    }
    addRandom(board, new_s);
    addRandom(board, new_s);
    drawBoard(board, new_s);
    score = 0;
}

void setBufferedInput(bool enable) {
    static bool enabled = true;
    static struct termios old;
    struct termios new;

    if (enable && !enabled) {
        // restore the former settings
        tcsetattr(STDIN_FILENO,TCSANOW,&old);
        // set the new state
        enabled = true;
    } else if (!enable && enabled) {
        // get the terminal settings for standard input
        tcgetattr(STDIN_FILENO,&new);
        // we want to keep the old setting to restore them at the end
        old = new;
        // disable canonical mode (buffered i/o) and local echo
        new.c_lflag &=(~ICANON & ~ECHO);
        // set the new settings immediately
        tcsetattr(STDIN_FILENO,TCSANOW,&new);
        // set the new state
        enabled = false;
    }
}

int test() {
    uint8_t array[SIZE];
    // these are exponents with base 2 (1=2 2=4 3=8)
    uint8_t data[] = {
        0,0,0,1,    1,0,0,0,
        0,0,1,1,    2,0,0,0,
        0,1,0,1,    2,0,0,0,
        1,0,0,1,    2,0,0,0,
        1,0,1,0,    2,0,0,0,
        1,1,1,0,    2,1,0,0,
        1,0,1,1,    2,1,0,0,
        1,1,0,1,    2,1,0,0,
        1,1,1,1,    2,2,0,0,
        2,2,1,1,    3,2,0,0,
        1,1,2,2,    2,3,0,0,
        3,0,1,1,    3,2,0,0,
        2,0,1,1,    2,2,0,0
    };
    uint8_t *in,*out;
    uint8_t t,tests;
    uint8_t i;
    bool success = true;

    tests = (sizeof(data)/sizeof(data[0]))/(2*SIZE);
    for (t=0;t<tests;t++) {
        in = data+t*2*SIZE;
        out = in + SIZE;
        for (i=0;i<SIZE;i++) {
            array[i] = in[i];
        }
        slideArray(array, SIZE);
        for (i=0;i<SIZE;i++) {
            if (array[i] != out[i]) {
                success = false;
            }
        }
        if (success==false) {
            for (i=0;i<SIZE;i++) {
                printf("%d ",in[i]);
            }
            printf("=> ");
            for (i=0;i<SIZE;i++) {
                printf("%d ",array[i]);
            }
            printf("expected ");
            for (i=0;i<SIZE;i++) {
                printf("%d ",in[i]);
            }
            printf("=> ");
            for (i=0;i<SIZE;i++) {
                printf("%d ",out[i]);
            }
            printf("\n");
            break;
        }
    }
    if (success) {
        printf("All %u tests executed successfully\n",tests);
    }
    return !success;
}

void signal_callback_handler(int signum) {
    printf("         TERMINATED         \n");
    setBufferedInput(true);
    printf("\e[?25h\e[m");
    exit(signum);
}

#if !HETERO_EN
struct slice_return main_loop_slice(char c, uint8_t board[SIZE][SIZE], 
    int new_s, llsp_t *restrict solver)
#elif HETERO_EN
struct slice_return main_loop_slice(char c, uint8_t board[SIZE][SIZE], 
    int new_s, llsp_t *restrict solver_big, llsp_t *restrict solver_little)
#endif
{
  uint8_t scheme_rename = scheme;
  uint8_t board_rename[SIZE][SIZE];
  int board_i0;
  for (board_i0 = 0; board_i0 < new_s; board_i0++)
  {
    int board_i1;
    for (board_i1 = 0; board_i1 < new_s; board_i1++)
    {
      board_rename[board_i0][board_i1] = board[board_i0][board_i1];
    }

  }

  int loop_counter[95] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int reduced_loop_counter[N_FEATURE] = {0};

  switch (c)
  {
    case 97:
      loop_counter[0]++;

    case 104:
      loop_counter[1]++;

    case 68:
      loop_counter[2]++;
    {
      bool return_value;
      bool success_rename0;
      {
        uint8_t i_rename5;
        uint8_t j_rename5;
        uint8_t n_rename5 = new_s;
        uint8_t tmp_rename5;
        for (i_rename5 = 0; i_rename5 < (n_rename5 / 2); i_rename5++)
        {
          loop_counter[3]++;
          for (j_rename5 = i_rename5; j_rename5 < ((n_rename5 - i_rename5) - 1); j_rename5++)
          {
            loop_counter[4]++;
            tmp_rename5 = board_rename[i_rename5][j_rename5];
            board_rename[i_rename5][j_rename5] = board_rename[j_rename5][(n_rename5 - i_rename5) - 1];
            board_rename[j_rename5][(n_rename5 - i_rename5) - 1] = board_rename[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1];
            board_rename[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1] = board_rename[(n_rename5 - j_rename5) - 1][i_rename5];
            board_rename[(n_rename5 - j_rename5) - 1][i_rename5] = tmp_rename5;
          }

        }

        return5:
        ;

      }
      {
        bool return_value;
        bool success_rename6 = false;
        uint8_t x_rename6;
        for (x_rename6 = 0; x_rename6 < new_s; x_rename6++)
        {
          loop_counter[5]++;
          {
            bool return_value;
            bool success_rename24 = false;
            uint8_t x_rename24;
            uint8_t t_rename24;
            uint8_t stop_rename24 = 0;
            for (x_rename24 = 0; x_rename24 < new_s; x_rename24++)
            {
              loop_counter[6]++;
              if (board_rename[x_rename6][x_rename24] != 0)
              {
                loop_counter[7]++;
                {
                  uint8_t return_value;
                  uint8_t stop_rename28 = stop_rename24;
                  uint8_t x_rename28 = x_rename24;
                  uint8_t t_rename28;
                  if (x_rename28 == 0)
                  {
                    loop_counter[8]++;
                    {
                      return_value = x_rename28;
                      goto return28;
                    }
                  }

                  for (t_rename28 = x_rename28 - 1; t_rename28 >= 0; t_rename28--)
                  {
                    loop_counter[9]++;
                    if (board_rename[x_rename6][t_rename28] != 0)
                    {
                      loop_counter[10]++;
                      if (board_rename[x_rename6][t_rename28] != board_rename[x_rename6][x_rename28])
                      {
                        loop_counter[11]++;
                        {
                          return_value = t_rename28 + 1;
                          goto return28;
                        }
                      }

                      {
                        return_value = t_rename28;
                        goto return28;
                      }
                    }
                    else
                    {
                      if (t_rename28 == stop_rename28)
                      {
                        loop_counter[12]++;
                        {
                          return_value = t_rename28;
                          goto return28;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename28;
                    goto return28;
                  }
                  return28:
                  ;

                  t_rename24 = return_value;
                }
                if (t_rename24 != x_rename24)
                {
                  loop_counter[13]++;
                  if (board_rename[x_rename6][t_rename24] == 0)
                  {
                    loop_counter[14]++;
                    board_rename[x_rename6][t_rename24] = board_rename[x_rename6][x_rename24];
                  }
                  else
                    if (board_rename[x_rename6][t_rename24] == board_rename[x_rename6][x_rename24])
                  {
                    loop_counter[15]++;
                    board_rename[x_rename6][t_rename24]++;
                    stop_rename24 = t_rename24 + 1;
                  }


                  board_rename[x_rename6][x_rename24] = 0;
                  success_rename24 = true;
                }

              }

            }

            {
              return_value = success_rename24;
              goto return24;
            }
            return24:
            ;

            success_rename6 = return_value;
          }
        }

        {
          return_value = success_rename6;
          goto return6;
        }
        return6:
        ;

        success_rename0 = return_value;
      }
      {
        uint8_t i_rename7;
        uint8_t j_rename7;
        uint8_t n_rename7 = new_s;
        uint8_t tmp_rename7;
        for (i_rename7 = 0; i_rename7 < (n_rename7 / 2); i_rename7++)
        {
          loop_counter[16]++;
          for (j_rename7 = i_rename7; j_rename7 < ((n_rename7 - i_rename7) - 1); j_rename7++)
          {
            loop_counter[17]++;
            tmp_rename7 = board_rename[i_rename7][j_rename7];
            board_rename[i_rename7][j_rename7] = board_rename[j_rename7][(n_rename7 - i_rename7) - 1];
            board_rename[j_rename7][(n_rename7 - i_rename7) - 1] = board_rename[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1];
            board_rename[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1] = board_rename[(n_rename7 - j_rename7) - 1][i_rename7];
            board_rename[(n_rename7 - j_rename7) - 1][i_rename7] = tmp_rename7;
          }

        }

        return7:
        ;

      }
      {
        uint8_t i_rename8;
        uint8_t j_rename8;
        uint8_t n_rename8 = new_s;
        uint8_t tmp_rename8;
        for (i_rename8 = 0; i_rename8 < (n_rename8 / 2); i_rename8++)
        {
          loop_counter[18]++;
          for (j_rename8 = i_rename8; j_rename8 < ((n_rename8 - i_rename8) - 1); j_rename8++)
          {
            loop_counter[19]++;
            tmp_rename8 = board_rename[i_rename8][j_rename8];
            board_rename[i_rename8][j_rename8] = board_rename[j_rename8][(n_rename8 - i_rename8) - 1];
            board_rename[j_rename8][(n_rename8 - i_rename8) - 1] = board_rename[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1];
            board_rename[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1] = board_rename[(n_rename8 - j_rename8) - 1][i_rename8];
            board_rename[(n_rename8 - j_rename8) - 1][i_rename8] = tmp_rename8;
          }

        }

        return8:
        ;

      }
      {
        uint8_t i_rename9;
        uint8_t j_rename9;
        uint8_t n_rename9 = new_s;
        uint8_t tmp_rename9;
        for (i_rename9 = 0; i_rename9 < (n_rename9 / 2); i_rename9++)
        {
          loop_counter[20]++;
          for (j_rename9 = i_rename9; j_rename9 < ((n_rename9 - i_rename9) - 1); j_rename9++)
          {
            loop_counter[21]++;
            tmp_rename9 = board_rename[i_rename9][j_rename9];
            board_rename[i_rename9][j_rename9] = board_rename[j_rename9][(n_rename9 - i_rename9) - 1];
            board_rename[j_rename9][(n_rename9 - i_rename9) - 1] = board_rename[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1];
            board_rename[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1] = board_rename[(n_rename9 - j_rename9) - 1][i_rename9];
            board_rename[(n_rename9 - j_rename9) - 1][i_rename9] = tmp_rename9;
          }

        }

        return9:
        ;

      }
      {
        return_value = success_rename0;
        goto return0;
      }
      return0:
      ;

    }
      break;

    case 100:
      loop_counter[22]++;

    case 108:
      loop_counter[23]++;

    case 67:
      loop_counter[24]++;
    {
      bool return_value;
      bool success_rename1;
      {
        uint8_t i_rename10;
        uint8_t j_rename10;
        uint8_t n_rename10 = new_s;
        uint8_t tmp_rename10;
        for (i_rename10 = 0; i_rename10 < (n_rename10 / 2); i_rename10++)
        {
          loop_counter[25]++;
          for (j_rename10 = i_rename10; j_rename10 < ((n_rename10 - i_rename10) - 1); j_rename10++)
          {
            loop_counter[26]++;
            tmp_rename10 = board_rename[i_rename10][j_rename10];
            board_rename[i_rename10][j_rename10] = board_rename[j_rename10][(n_rename10 - i_rename10) - 1];
            board_rename[j_rename10][(n_rename10 - i_rename10) - 1] = board_rename[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1];
            board_rename[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1] = board_rename[(n_rename10 - j_rename10) - 1][i_rename10];
            board_rename[(n_rename10 - j_rename10) - 1][i_rename10] = tmp_rename10;
          }

        }

        return10:
        ;

      }
      {
        uint8_t i_rename11;
        uint8_t j_rename11;
        uint8_t n_rename11 = new_s;
        uint8_t tmp_rename11;
        for (i_rename11 = 0; i_rename11 < (n_rename11 / 2); i_rename11++)
        {
          loop_counter[27]++;
          for (j_rename11 = i_rename11; j_rename11 < ((n_rename11 - i_rename11) - 1); j_rename11++)
          {
            loop_counter[28]++;
            tmp_rename11 = board_rename[i_rename11][j_rename11];
            board_rename[i_rename11][j_rename11] = board_rename[j_rename11][(n_rename11 - i_rename11) - 1];
            board_rename[j_rename11][(n_rename11 - i_rename11) - 1] = board_rename[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1];
            board_rename[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1] = board_rename[(n_rename11 - j_rename11) - 1][i_rename11];
            board_rename[(n_rename11 - j_rename11) - 1][i_rename11] = tmp_rename11;
          }

        }

        return11:
        ;

      }
      {
        uint8_t i_rename12;
        uint8_t j_rename12;
        uint8_t n_rename12 = new_s;
        uint8_t tmp_rename12;
        for (i_rename12 = 0; i_rename12 < (n_rename12 / 2); i_rename12++)
        {
          loop_counter[29]++;
          for (j_rename12 = i_rename12; j_rename12 < ((n_rename12 - i_rename12) - 1); j_rename12++)
          {
            loop_counter[30]++;
            tmp_rename12 = board_rename[i_rename12][j_rename12];
            board_rename[i_rename12][j_rename12] = board_rename[j_rename12][(n_rename12 - i_rename12) - 1];
            board_rename[j_rename12][(n_rename12 - i_rename12) - 1] = board_rename[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1];
            board_rename[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1] = board_rename[(n_rename12 - j_rename12) - 1][i_rename12];
            board_rename[(n_rename12 - j_rename12) - 1][i_rename12] = tmp_rename12;
          }

        }

        return12:
        ;

      }
      {
        bool return_value;
        bool success_rename13 = false;
        uint8_t x_rename13;
        for (x_rename13 = 0; x_rename13 < new_s; x_rename13++)
        {
          loop_counter[31]++;
          {
            bool return_value;
            bool success_rename25 = false;
            uint8_t x_rename25;
            uint8_t t_rename25;
            uint8_t stop_rename25 = 0;
            for (x_rename25 = 0; x_rename25 < new_s; x_rename25++)
            {
              loop_counter[32]++;
              if (board_rename[x_rename13][x_rename25] != 0)
              {
                loop_counter[33]++;
                {
                  uint8_t return_value;
                  uint8_t stop_rename29 = stop_rename25;
                  uint8_t x_rename29 = x_rename25;
                  uint8_t t_rename29;
                  if (x_rename29 == 0)
                  {
                    loop_counter[34]++;
                    {
                      return_value = x_rename29;
                      goto return29;
                    }
                  }

                  for (t_rename29 = x_rename29 - 1; t_rename29 >= 0; t_rename29--)
                  {
                    loop_counter[35]++;
                    if (board_rename[x_rename13][t_rename29] != 0)
                    {
                      loop_counter[36]++;
                      if (board_rename[x_rename13][t_rename29] != board_rename[x_rename13][x_rename29])
                      {
                        loop_counter[37]++;
                        {
                          return_value = t_rename29 + 1;
                          goto return29;
                        }
                      }

                      {
                        return_value = t_rename29;
                        goto return29;
                      }
                    }
                    else
                    {
                      if (t_rename29 == stop_rename29)
                      {
                        loop_counter[38]++;
                        {
                          return_value = t_rename29;
                          goto return29;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename29;
                    goto return29;
                  }
                  return29:
                  ;

                  t_rename25 = return_value;
                }
                if (t_rename25 != x_rename25)
                {
                  loop_counter[39]++;
                  if (board_rename[x_rename13][t_rename25] == 0)
                  {
                    loop_counter[40]++;
                    board_rename[x_rename13][t_rename25] = board_rename[x_rename13][x_rename25];
                  }
                  else
                    if (board_rename[x_rename13][t_rename25] == board_rename[x_rename13][x_rename25])
                  {
                    loop_counter[41]++;
                    board_rename[x_rename13][t_rename25]++;
                    stop_rename25 = t_rename25 + 1;
                  }


                  board_rename[x_rename13][x_rename25] = 0;
                  success_rename25 = true;
                }

              }

            }

            {
              return_value = success_rename25;
              goto return25;
            }
            return25:
            ;

            success_rename13 = return_value;
          }
        }

        {
          return_value = success_rename13;
          goto return13;
        }
        return13:
        ;

        success_rename1 = return_value;
      }
      {
        uint8_t i_rename14;
        uint8_t j_rename14;
        uint8_t n_rename14 = new_s;
        uint8_t tmp_rename14;
        for (i_rename14 = 0; i_rename14 < (n_rename14 / 2); i_rename14++)
        {
          loop_counter[42]++;
          for (j_rename14 = i_rename14; j_rename14 < ((n_rename14 - i_rename14) - 1); j_rename14++)
          {
            loop_counter[43]++;
            tmp_rename14 = board_rename[i_rename14][j_rename14];
            board_rename[i_rename14][j_rename14] = board_rename[j_rename14][(n_rename14 - i_rename14) - 1];
            board_rename[j_rename14][(n_rename14 - i_rename14) - 1] = board_rename[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1];
            board_rename[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1] = board_rename[(n_rename14 - j_rename14) - 1][i_rename14];
            board_rename[(n_rename14 - j_rename14) - 1][i_rename14] = tmp_rename14;
          }

        }

        return14:
        ;

      }
      {
        return_value = success_rename1;
        goto return1;
      }
      return1:
      ;

    }
      break;

    case 119:
      loop_counter[44]++;

    case 107:
      loop_counter[45]++;

    case 65:
      loop_counter[46]++;
    {
      bool return_value;
      bool success_rename2 = false;
      uint8_t x_rename2;
      for (x_rename2 = 0; x_rename2 < new_s; x_rename2++)
      {
        loop_counter[47]++;
        {
          bool return_value;
          bool success_rename15 = false;
          uint8_t x_rename15;
          uint8_t t_rename15;
          uint8_t stop_rename15 = 0;
          for (x_rename15 = 0; x_rename15 < new_s; x_rename15++)
          {
            loop_counter[48]++;
            if (board_rename[x_rename2][x_rename15] != 0)
            {
              loop_counter[49]++;
              {
                uint8_t return_value;
                uint8_t stop_rename26 = stop_rename15;
                uint8_t x_rename26 = x_rename15;
                uint8_t t_rename26;
                if (x_rename26 == 0)
                {
                  loop_counter[50]++;
                  {
                    return_value = x_rename26;
                    goto return26;
                  }
                }

                for (t_rename26 = x_rename26 - 1; t_rename26 >= 0; t_rename26--)
                {
                  loop_counter[51]++;
                  if (board_rename[x_rename2][t_rename26] != 0)
                  {
                    loop_counter[52]++;
                    if (board_rename[x_rename2][t_rename26] != board_rename[x_rename2][x_rename26])
                    {
                      loop_counter[53]++;
                      {
                        return_value = t_rename26 + 1;
                        goto return26;
                      }
                    }

                    {
                      return_value = t_rename26;
                      goto return26;
                    }
                  }
                  else
                  {
                    if (t_rename26 == stop_rename26)
                    {
                      loop_counter[54]++;
                      {
                        return_value = t_rename26;
                        goto return26;
                      }
                    }

                  }

                }

                {
                  return_value = x_rename26;
                  goto return26;
                }
                return26:
                ;

                t_rename15 = return_value;
              }
              if (t_rename15 != x_rename15)
              {
                loop_counter[55]++;
                if (board_rename[x_rename2][t_rename15] == 0)
                {
                  loop_counter[56]++;
                  board_rename[x_rename2][t_rename15] = board_rename[x_rename2][x_rename15];
                }
                else
                  if (board_rename[x_rename2][t_rename15] == board_rename[x_rename2][x_rename15])
                {
                  loop_counter[57]++;
                  board_rename[x_rename2][t_rename15]++;
                  stop_rename15 = t_rename15 + 1;
                }


                board_rename[x_rename2][x_rename15] = 0;
                success_rename15 = true;
              }

            }

          }

          {
            return_value = success_rename15;
            goto return15;
          }
          return15:
          ;

          success_rename2 = return_value;
        }
      }

      {
        return_value = success_rename2;
        goto return2;
      }
      return2:
      ;

    }
      break;

    case 115:
      loop_counter[58]++;

    case 106:
      loop_counter[59]++;

    case 66:
      loop_counter[60]++;
    {
      bool return_value;
      bool success_rename3;
      {
        uint8_t i_rename16;
        uint8_t j_rename16;
        uint8_t n_rename16 = new_s;
        uint8_t tmp_rename16;
        for (i_rename16 = 0; i_rename16 < (n_rename16 / 2); i_rename16++)
        {
          loop_counter[61]++;
          for (j_rename16 = i_rename16; j_rename16 < ((n_rename16 - i_rename16) - 1); j_rename16++)
          {
            loop_counter[62]++;
            tmp_rename16 = board_rename[i_rename16][j_rename16];
            board_rename[i_rename16][j_rename16] = board_rename[j_rename16][(n_rename16 - i_rename16) - 1];
            board_rename[j_rename16][(n_rename16 - i_rename16) - 1] = board_rename[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1];
            board_rename[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1] = board_rename[(n_rename16 - j_rename16) - 1][i_rename16];
            board_rename[(n_rename16 - j_rename16) - 1][i_rename16] = tmp_rename16;
          }

        }

        return16:
        ;

      }
      {
        uint8_t i_rename17;
        uint8_t j_rename17;
        uint8_t n_rename17 = new_s;
        uint8_t tmp_rename17;
        for (i_rename17 = 0; i_rename17 < (n_rename17 / 2); i_rename17++)
        {
          loop_counter[63]++;
          for (j_rename17 = i_rename17; j_rename17 < ((n_rename17 - i_rename17) - 1); j_rename17++)
          {
            loop_counter[64]++;
            tmp_rename17 = board_rename[i_rename17][j_rename17];
            board_rename[i_rename17][j_rename17] = board_rename[j_rename17][(n_rename17 - i_rename17) - 1];
            board_rename[j_rename17][(n_rename17 - i_rename17) - 1] = board_rename[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1];
            board_rename[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1] = board_rename[(n_rename17 - j_rename17) - 1][i_rename17];
            board_rename[(n_rename17 - j_rename17) - 1][i_rename17] = tmp_rename17;
          }

        }

        return17:
        ;

      }
      {
        bool return_value;
        bool success_rename18 = false;
        uint8_t x_rename18;
        for (x_rename18 = 0; x_rename18 < 4; x_rename18++)
        {
          loop_counter[65]++;
          {
            bool return_value;
            bool success_rename27 = false;
            uint8_t x_rename27;
            uint8_t t_rename27;
            uint8_t stop_rename27 = 0;
            for (x_rename27 = 0; x_rename27 < 4; x_rename27++)
            {
              loop_counter[66]++;
              if (board_rename[x_rename18][x_rename27] != 0)
              {
                loop_counter[67]++;
                {
                  uint8_t return_value;
                  uint8_t stop_rename30 = stop_rename27;
                  uint8_t x_rename30 = x_rename27;
                  uint8_t t_rename30;
                  if (x_rename30 == 0)
                  {
                    loop_counter[68]++;
                    {
                      return_value = x_rename30;
                      goto return30;
                    }
                  }

                  for (t_rename30 = x_rename30 - 1; t_rename30 >= 0; t_rename30--)
                  {
                    loop_counter[69]++;
                    if (board_rename[x_rename18][t_rename30] != 0)
                    {
                      loop_counter[70]++;
                      if (board_rename[x_rename18][t_rename30] != board_rename[x_rename18][x_rename30])
                      {
                        loop_counter[71]++;
                        {
                          return_value = t_rename30 + 1;
                          goto return30;
                        }
                      }

                      {
                        return_value = t_rename30;
                        goto return30;
                      }
                    }
                    else
                    {
                      if (t_rename30 == stop_rename30)
                      {
                        loop_counter[72]++;
                        {
                          return_value = t_rename30;
                          goto return30;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename30;
                    goto return30;
                  }
                  return30:
                  ;

                  t_rename27 = return_value;
                }
                if (t_rename27 != x_rename27)
                {
                  loop_counter[73]++;
                  if (board_rename[x_rename18][t_rename27] == 0)
                  {
                    loop_counter[74]++;
                    board_rename[x_rename18][t_rename27] = board_rename[x_rename18][x_rename27];
                  }
                  else
                    if (board_rename[x_rename18][t_rename27] == board_rename[x_rename18][x_rename27])
                  {
                    loop_counter[75]++;
                    board_rename[x_rename18][t_rename27]++;
                    stop_rename27 = t_rename27 + 1;
                  }


                  board_rename[x_rename18][x_rename27] = 0;
                  success_rename27 = true;
                }

              }

            }

            {
              return_value = success_rename27;
              goto return27;
            }
            return27:
            ;

            success_rename18 = return_value;
          }
        }

        {
          return_value = success_rename18;
          goto return18;
        }
        return18:
        ;

        success_rename3 = return_value;
      }
      {
        uint8_t i_rename19;
        uint8_t j_rename19;
        uint8_t n_rename19 = new_s;
        uint8_t tmp_rename19;
        for (i_rename19 = 0; i_rename19 < (n_rename19 / 2); i_rename19++)
        {
          loop_counter[76]++;
          for (j_rename19 = i_rename19; j_rename19 < ((n_rename19 - i_rename19) - 1); j_rename19++)
          {
            loop_counter[77]++;
            tmp_rename19 = board_rename[i_rename19][j_rename19];
            board_rename[i_rename19][j_rename19] = board_rename[j_rename19][(n_rename19 - i_rename19) - 1];
            board_rename[j_rename19][(n_rename19 - i_rename19) - 1] = board_rename[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1];
            board_rename[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1] = board_rename[(n_rename19 - j_rename19) - 1][i_rename19];
            board_rename[(n_rename19 - j_rename19) - 1][i_rename19] = tmp_rename19;
          }

        }

        return19:
        ;

      }
      {
        uint8_t i_rename20;
        uint8_t j_rename20;
        uint8_t n_rename20 = new_s;
        uint8_t tmp_rename20;
        for (i_rename20 = 0; i_rename20 < (n_rename20 / 2); i_rename20++)
        {
          loop_counter[78]++;
          for (j_rename20 = i_rename20; j_rename20 < ((n_rename20 - i_rename20) - 1); j_rename20++)
          {
            loop_counter[79]++;
            tmp_rename20 = board_rename[i_rename20][j_rename20];
            board_rename[i_rename20][j_rename20] = board_rename[j_rename20][(n_rename20 - i_rename20) - 1];
            board_rename[j_rename20][(n_rename20 - i_rename20) - 1] = board_rename[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1];
            board_rename[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1] = board_rename[(n_rename20 - j_rename20) - 1][i_rename20];
            board_rename[(n_rename20 - j_rename20) - 1][i_rename20] = tmp_rename20;
          }

        }

        return20:
        ;

      }
      {
        return_value = success_rename3;
        goto return3;
      }
      return3:
      ;

    }
      break;

    default:
      loop_counter[80]++;

  }

  {
    uint8_t x_rename4;
    uint8_t y_rename4;
    char color_rename4[40];
    for (y_rename4 = 0; y_rename4 < new_s; y_rename4++)
    {
      loop_counter[81]++;
      for (x_rename4 = 0; x_rename4 < new_s; x_rename4++)
      {
        loop_counter[82]++;
        {
          size_t length_rename21 = 40;
          uint8_t value_rename21 = board_rename[x_rename4][y_rename4];
          uint8_t original_rename21[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
          uint8_t blackwhite_rename21[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
          uint8_t bluered_rename21[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
          uint8_t *schemes_rename21[] = {original_rename21, blackwhite_rename21, bluered_rename21};
          uint8_t *background_rename21 = schemes_rename21[scheme_rename] + 0;
          uint8_t *foreground_rename21 = schemes_rename21[scheme_rename] + 1;
          if (value_rename21 > 0)
          {
            loop_counter[83]++;
            while (value_rename21--)
            {
              loop_counter[84]++;
              if ((background_rename21 + 2) < (schemes_rename21[scheme_rename] + (sizeof(original_rename21))))
              {
                loop_counter[85]++;
                background_rename21 += 2;
                foreground_rename21 += 2;
              }

            }

          }

          snprintf(color_rename4, length_rename21, "\e[38;5;%d;48;5;%dm", *foreground_rename21, *background_rename21);
          return21:
          ;

        }
      }

      for (x_rename4 = 0; x_rename4 < new_s; x_rename4++)
      {
        loop_counter[86]++;
        {
          size_t length_rename22 = 40;
          uint8_t value_rename22 = board_rename[x_rename4][y_rename4];
          uint8_t original_rename22[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
          uint8_t blackwhite_rename22[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
          uint8_t bluered_rename22[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
          uint8_t *schemes_rename22[] = {original_rename22, blackwhite_rename22, bluered_rename22};
          uint8_t *background_rename22 = schemes_rename22[scheme_rename] + 0;
          uint8_t *foreground_rename22 = schemes_rename22[scheme_rename] + 1;
          if (value_rename22 > 0)
          {
            loop_counter[87]++;
            while (value_rename22--)
            {
              loop_counter[88]++;
              if ((background_rename22 + 2) < (schemes_rename22[scheme_rename] + (sizeof(original_rename22))))
              {
                loop_counter[89]++;
                background_rename22 += 2;
                foreground_rename22 += 2;
              }

            }

          }

          snprintf(color_rename4, length_rename22, "\e[38;5;%d;48;5;%dm", *foreground_rename22, *background_rename22);
          return22:
          ;

        }
        if (board_rename[x_rename4][y_rename4] != 0)
        {
          loop_counter[90]++;
        }
        else
        {
        }

      }

      for (x_rename4 = 0; x_rename4 < new_s; x_rename4++)
      {
        loop_counter[91]++;
        {
          size_t length_rename23 = 40;
          uint8_t value_rename23 = board_rename[x_rename4][y_rename4];
          uint8_t original_rename23[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
          uint8_t blackwhite_rename23[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
          uint8_t bluered_rename23[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
          uint8_t *schemes_rename23[] = {original_rename23, blackwhite_rename23, bluered_rename23};
          uint8_t *background_rename23 = schemes_rename23[scheme_rename] + 0;
          uint8_t *foreground_rename23 = schemes_rename23[scheme_rename] + 1;
          if (value_rename23 > 0)
          {
            loop_counter[92]++;
            while (value_rename23--)
            {
              loop_counter[93]++;
              if ((background_rename23 + 2) < (schemes_rename23[scheme_rename] + (sizeof(original_rename23))))
              {
                loop_counter[94]++;
                background_rename23 += 2;
                foreground_rename23 += 2;
              }

            }

          }

          snprintf(color_rename4, length_rename23, "\e[38;5;%d;48;5;%dm", *foreground_rename23, *background_rename23);
          return23:
          ;

        }
      }

    }

    return4:
    ;

  }
  {
    goto print_loop_counter;
  }
  {
    print_loop_counter:
    ;
#if GET_PREDICT || DEBUG_EN
    //95
    print_array(loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]));
// non-zero coeffs =  [2, 3, 4, 5, 6, 16, 17, 18, 19, 20, 21, 24, 25, 26,
// 27, 28, 29, 30, 31, 32, 42, 43, 46, 47, 48, 60, 61, 62, 63, 64, 65, 66,
// 76, 77, 78, 79, 81, 82, 86, 91]
    reduced_loop_counter[0] = loop_counter[2];
    reduced_loop_counter[1] = loop_counter[3];
    reduced_loop_counter[2] = loop_counter[4];
    reduced_loop_counter[3] = loop_counter[5];
    reduced_loop_counter[4] = loop_counter[6];
    reduced_loop_counter[5] = loop_counter[16];
    reduced_loop_counter[6] = loop_counter[17];
    reduced_loop_counter[7] = loop_counter[18];
    reduced_loop_counter[8] = loop_counter[19];
    reduced_loop_counter[9] = loop_counter[20];
    reduced_loop_counter[10] = loop_counter[21];
    reduced_loop_counter[11] = loop_counter[24];
    reduced_loop_counter[12] = loop_counter[25];
    reduced_loop_counter[13] = loop_counter[26];
    reduced_loop_counter[14] = loop_counter[27];
    reduced_loop_counter[15] = loop_counter[28];
    reduced_loop_counter[16] = loop_counter[29];
    reduced_loop_counter[17] = loop_counter[30];
    reduced_loop_counter[18] = loop_counter[31];
    reduced_loop_counter[19] = loop_counter[32];
    reduced_loop_counter[20] = loop_counter[42];
    reduced_loop_counter[21] = loop_counter[43];
    reduced_loop_counter[22] = loop_counter[46];
    reduced_loop_counter[23] = loop_counter[47];
    reduced_loop_counter[24] = loop_counter[48];
    reduced_loop_counter[25] = loop_counter[60];
    reduced_loop_counter[26] = loop_counter[61];
    reduced_loop_counter[27] = loop_counter[62];
    reduced_loop_counter[28] = loop_counter[63];
    reduced_loop_counter[29] = loop_counter[64];
    reduced_loop_counter[30] = loop_counter[65];
    reduced_loop_counter[31] = loop_counter[66];
    reduced_loop_counter[32] = loop_counter[76];
    reduced_loop_counter[33] = loop_counter[77];
    reduced_loop_counter[34] = loop_counter[78];
    reduced_loop_counter[35] = loop_counter[79];
    reduced_loop_counter[36] = loop_counter[81];
    reduced_loop_counter[37] = loop_counter[82];
    reduced_loop_counter[38] = loop_counter[86];
    reduced_loop_counter[39] = loop_counter[91];
#endif
  }
  {
    predict_exec_time:
    ;
    struct slice_return exec_time;
    exec_time.big = exec_time.little = 0; //initialize
#if !ONLINE_EN
  #if !CVX_EN //off-line training with conservative
    #if ARCH_ARM
      exec_time.little = 1221.939950*loop_counter[2] + 49.304312*loop_counter[3] + 23.786664*loop_counter[4] + -28.282174*loop_counter[5] + 7.707451*loop_counter[6] + 49.304309*loop_counter[16] + 23.786664*loop_counter[17] + 49.304309*loop_counter[18] + 23.786664*loop_counter[19] + 49.304309*loop_counter[20] + 23.786664*loop_counter[21] + 1958.943525*loop_counter[24] + -220.179373*loop_counter[25] + 25.300919*loop_counter[26] + -220.179371*loop_counter[27] + 25.300919*loop_counter[28] + -220.179371*loop_counter[29] + 25.300919*loop_counter[30] + 299.217046*loop_counter[31] + 13.856857*loop_counter[32] + -220.179368*loop_counter[42] + 25.300920*loop_counter[43] + 1394.525755*loop_counter[46] + 14.233542*loop_counter[47] + 33.584326*loop_counter[48] + 312.115881*loop_counter[60] + 9.074714*loop_counter[61] + 33.779080*loop_counter[62] + 9.074715*loop_counter[63] + 33.779080*loop_counter[64] + 110.349632*loop_counter[65] + 39.014483*loop_counter[66] + 9.074715*loop_counter[76] + 33.779080*loop_counter[77] + 9.074714*loop_counter[78] + 33.779081*loop_counter[79] + -1730.032271*loop_counter[81] + 43.684433*loop_counter[82] + 43.684433*loop_counter[86] + 43.684433*loop_counter[91] + 4731.467142;
    #elif ARCH_X86
      exec_time.little = 6.184759*loop_counter[2] + 7.020581*loop_counter[3] +
        0.045134*loop_counter[4] + 4.174456*loop_counter[5] +
        0.015958*loop_counter[6] + 7.020581*loop_counter[16] +
        0.045134*loop_counter[17] + 7.020581*loop_counter[18] +
        0.045133*loop_counter[19] + 7.020581*loop_counter[20] +
        0.045133*loop_counter[21] + 7.184752*loop_counter[24] +
        6.908165*loop_counter[25] + 0.052830*loop_counter[26] +
        6.908163*loop_counter[27] + 0.052830*loop_counter[28] +
        6.908163*loop_counter[29] + 0.052830*loop_counter[30] +
        4.107625*loop_counter[31] + 0.018678*loop_counter[32] +
        6.908164*loop_counter[42] + 0.052830*loop_counter[43] +
        14.538062*loop_counter[46] + 14.813545*loop_counter[47] +
        0.202028*loop_counter[48] + -0.184669*loop_counter[60] +
        9.566143*loop_counter[61] + 0.019425*loop_counter[62] +
        9.566142*loop_counter[63] + 0.019425*loop_counter[64] +
        -0.065290*loop_counter[65] + -0.023088*loop_counter[66] +
        9.566142*loop_counter[76] + 0.019425*loop_counter[77] +
        9.566142*loop_counter[78] + 0.019425*loop_counter[79] +
        27.117715*loop_counter[81] + 0.243525*loop_counter[82] +
        0.243525*loop_counter[86] + 0.243525*loop_counter[91] + 27.815240;
    #endif
  #else //off-line training with cvx    
    #if ARCH_ARM
      exec_time.little = 1623.486563*loop_counter[2] + -42.893065*loop_counter[3] + 16.286125*loop_counter[4] + -577.936322*loop_counter[5] + -0.663678*loop_counter[6] + -42.893062*loop_counter[16] + 16.286125*loop_counter[17] + -42.893062*loop_counter[18] + 16.286125*loop_counter[19] + -42.893062*loop_counter[20] + 16.286125*loop_counter[21] + 1438.526563*loop_counter[24] + 5.092514*loop_counter[25] + 15.314973*loop_counter[26] + 5.092511*loop_counter[27] + 15.314973*loop_counter[28] + 5.092511*loop_counter[29] + 15.314973*loop_counter[30] + -623.027475*loop_counter[31] + -1.852525*loop_counter[32] + 5.092520*loop_counter[42] + 15.314970*loop_counter[43] + 1316.993230*loop_counter[46] + -580.442447*loop_counter[47] + 11.895781*loop_counter[48] + 1278.933622*loop_counter[60] + -984.961214*loop_counter[61] + 88.422447*loop_counter[62] + -984.961227*loop_counter[63] + 88.422447*loop_counter[64] + 452.171295*loop_counter[65] + 159.866735*loop_counter[66] + -984.961226*loop_counter[76] + 88.422448*loop_counter[77] + -984.961228*loop_counter[78] + 88.422448*loop_counter[79] + -1161.257553*loop_counter[81] + 51.979184*loop_counter[82] + 51.979184*loop_counter[86] + 51.979184*loop_counter[91] + 5018.473437;
    #elif ARCH_X86
      exec_time.little = 0;
    #endif
  #endif
#elif ONLINE_EN
  #if !HETERO_EN
    #if CORE //on-line training on big core
      exec_time.big    = get_predicted_time(TYPE_PREDICT, solver, loop_counter,
          sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    #else //on-line training on little core
      exec_time.little = get_predicted_time(TYPE_PREDICT, solver, loop_counter,
          sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    #endif
  #elif HETERO_EN
    exec_time.big    = get_predicted_time_big(TYPE_PREDICT, solver_big,
        loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
    exec_time.little = get_predicted_time_little(TYPE_PREDICT, solver_little,
        loop_counter, sizeof(loop_counter)/sizeof(loop_counter[0]), 0, 0);
  #endif
#endif
    return exec_time;
  }
}

/*
#if CORE //big
float main_loop_slice_reduced(char c, uint8_t board[4][4])
{
  uint8_t board_rename[4][4];
  int board_i0;
  for (board_i0 = 0; board_i0 < 4; board_i0++)
  {
    int board_i1;
    for (board_i1 = 0; board_i1 < 4; board_i1++)
    {
      board_rename[board_i0][board_i1] = board[board_i0][board_i1];
    }

  }

  int loop_counter[95] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  switch (c)
  {
    case 68:
      loop_counter[2]++;
    {
      bool return_value;
      bool success_rename0;
      {
        uint8_t i_rename5;
        uint8_t j_rename5;
        uint8_t n_rename5 = 4;
        uint8_t tmp_rename5;
        for (i_rename5 = 0; i_rename5 < (n_rename5 / 2); i_rename5++)
        {
          for (j_rename5 = i_rename5; j_rename5 < ((n_rename5 - i_rename5) - 1); j_rename5++)
          {
            tmp_rename5 = board_rename[i_rename5][j_rename5];
            board_rename[i_rename5][j_rename5] = board_rename[j_rename5][(n_rename5 - i_rename5) - 1];
            board_rename[j_rename5][(n_rename5 - i_rename5) - 1] = board_rename[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1];
            board_rename[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1] = board_rename[(n_rename5 - j_rename5) - 1][i_rename5];
            board_rename[(n_rename5 - j_rename5) - 1][i_rename5] = tmp_rename5;
          }

        }

        return5:
        ;

      }
      {
        bool return_value;
        bool success_rename6 = false;
        uint8_t x_rename6;
        for (x_rename6 = 0; x_rename6 < 4; x_rename6++)
        {
          {
            bool return_value;
            bool success_rename24 = false;
            uint8_t x_rename24;
            uint8_t t_rename24;
            uint8_t stop_rename24 = 0;
            for (x_rename24 = 0; x_rename24 < 4; x_rename24++)
            {
              if (board_rename[x_rename6][x_rename24] != 0)
              {
                {
                  uint8_t return_value;
                  uint8_t stop_rename28 = stop_rename24;
                  uint8_t x_rename28 = x_rename24;
                  uint8_t t_rename28;
                  if (x_rename28 == 0)
                  {
                    loop_counter[8]++;
                    {
                      return_value = x_rename28;
                      goto return28;
                    }
                  }

                  for (t_rename28 = x_rename28 - 1; t_rename28 >= 0; t_rename28--)
                  {
                    loop_counter[9]++;
                    if (board_rename[x_rename6][t_rename28] != 0)
                    {
                      loop_counter[10]++;
                      if (board_rename[x_rename6][t_rename28] != board_rename[x_rename6][x_rename28])
                      {
                        {
                          return_value = t_rename28 + 1;
                          goto return28;
                        }
                      }

                      {
                        return_value = t_rename28;
                        goto return28;
                      }
                    }
                    else
                    {
                      if (t_rename28 == stop_rename28)
                      {
                        loop_counter[12]++;
                        {
                          return_value = t_rename28;
                          goto return28;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename28;
                    goto return28;
                  }
                  return28:
                  ;

                  t_rename24 = return_value;
                }
                if (t_rename24 != x_rename24)
                {
                  if (board_rename[x_rename6][t_rename24] == 0)
                  {
                    loop_counter[14]++;
                    board_rename[x_rename6][t_rename24] = board_rename[x_rename6][x_rename24];
                  }
                  else
                    if (board_rename[x_rename6][t_rename24] == board_rename[x_rename6][x_rename24])
                  {
                    loop_counter[15]++;
                    board_rename[x_rename6][t_rename24]++;
                    stop_rename24 = t_rename24 + 1;
                  }


                  board_rename[x_rename6][x_rename24] = 0;
                  success_rename24 = true;
                }

              }

            }

            {
              return_value = success_rename24;
              goto return24;
            }
            return24:
            ;

            success_rename6 = return_value;
          }
        }

        {
          return_value = success_rename6;
          goto return6;
        }
        return6:
        ;

        success_rename0 = return_value;
      }
      {
        uint8_t i_rename7;
        uint8_t j_rename7;
        uint8_t n_rename7 = 4;
        uint8_t tmp_rename7;
        for (i_rename7 = 0; i_rename7 < (n_rename7 / 2); i_rename7++)
        {
          for (j_rename7 = i_rename7; j_rename7 < ((n_rename7 - i_rename7) - 1); j_rename7++)
          {
            tmp_rename7 = board_rename[i_rename7][j_rename7];
            board_rename[i_rename7][j_rename7] = board_rename[j_rename7][(n_rename7 - i_rename7) - 1];
            board_rename[j_rename7][(n_rename7 - i_rename7) - 1] = board_rename[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1];
            board_rename[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1] = board_rename[(n_rename7 - j_rename7) - 1][i_rename7];
            board_rename[(n_rename7 - j_rename7) - 1][i_rename7] = tmp_rename7;
          }

        }

        return7:
        ;

      }
      {
        uint8_t i_rename8;
        uint8_t j_rename8;
        uint8_t n_rename8 = 4;
        uint8_t tmp_rename8;
        for (i_rename8 = 0; i_rename8 < (n_rename8 / 2); i_rename8++)
        {
          for (j_rename8 = i_rename8; j_rename8 < ((n_rename8 - i_rename8) - 1); j_rename8++)
          {
            tmp_rename8 = board_rename[i_rename8][j_rename8];
            board_rename[i_rename8][j_rename8] = board_rename[j_rename8][(n_rename8 - i_rename8) - 1];
            board_rename[j_rename8][(n_rename8 - i_rename8) - 1] = board_rename[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1];
            board_rename[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1] = board_rename[(n_rename8 - j_rename8) - 1][i_rename8];
            board_rename[(n_rename8 - j_rename8) - 1][i_rename8] = tmp_rename8;
          }

        }

        return8:
        ;

      }
      {
        uint8_t i_rename9;
        uint8_t j_rename9;
        uint8_t n_rename9 = 4;
        uint8_t tmp_rename9;
        for (i_rename9 = 0; i_rename9 < (n_rename9 / 2); i_rename9++)
        {
          for (j_rename9 = i_rename9; j_rename9 < ((n_rename9 - i_rename9) - 1); j_rename9++)
          {
            tmp_rename9 = board_rename[i_rename9][j_rename9];
            board_rename[i_rename9][j_rename9] = board_rename[j_rename9][(n_rename9 - i_rename9) - 1];
            board_rename[j_rename9][(n_rename9 - i_rename9) - 1] = board_rename[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1];
            board_rename[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1] = board_rename[(n_rename9 - j_rename9) - 1][i_rename9];
            board_rename[(n_rename9 - j_rename9) - 1][i_rename9] = tmp_rename9;
          }

        }

        return9:
        ;

      }
      {
        return_value = success_rename0;
        goto return0;
      }
      return0:
      ;

    }
      break;

    case 67:
      loop_counter[24]++;
    {
      bool return_value;
      bool success_rename1;
      {
        uint8_t i_rename10;
        uint8_t j_rename10;
        uint8_t n_rename10 = 4;
        uint8_t tmp_rename10;
        for (i_rename10 = 0; i_rename10 < (n_rename10 / 2); i_rename10++)
        {
          for (j_rename10 = i_rename10; j_rename10 < ((n_rename10 - i_rename10) - 1); j_rename10++)
          {
            tmp_rename10 = board_rename[i_rename10][j_rename10];
            board_rename[i_rename10][j_rename10] = board_rename[j_rename10][(n_rename10 - i_rename10) - 1];
            board_rename[j_rename10][(n_rename10 - i_rename10) - 1] = board_rename[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1];
            board_rename[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1] = board_rename[(n_rename10 - j_rename10) - 1][i_rename10];
            board_rename[(n_rename10 - j_rename10) - 1][i_rename10] = tmp_rename10;
          }

        }

        return10:
        ;

      }
      {
        uint8_t i_rename11;
        uint8_t j_rename11;
        uint8_t n_rename11 = 4;
        uint8_t tmp_rename11;
        for (i_rename11 = 0; i_rename11 < (n_rename11 / 2); i_rename11++)
        {
          for (j_rename11 = i_rename11; j_rename11 < ((n_rename11 - i_rename11) - 1); j_rename11++)
          {
            tmp_rename11 = board_rename[i_rename11][j_rename11];
            board_rename[i_rename11][j_rename11] = board_rename[j_rename11][(n_rename11 - i_rename11) - 1];
            board_rename[j_rename11][(n_rename11 - i_rename11) - 1] = board_rename[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1];
            board_rename[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1] = board_rename[(n_rename11 - j_rename11) - 1][i_rename11];
            board_rename[(n_rename11 - j_rename11) - 1][i_rename11] = tmp_rename11;
          }

        }

        return11:
        ;

      }
      {
        uint8_t i_rename12;
        uint8_t j_rename12;
        uint8_t n_rename12 = 4;
        uint8_t tmp_rename12;
        for (i_rename12 = 0; i_rename12 < (n_rename12 / 2); i_rename12++)
        {
          for (j_rename12 = i_rename12; j_rename12 < ((n_rename12 - i_rename12) - 1); j_rename12++)
          {
            tmp_rename12 = board_rename[i_rename12][j_rename12];
            board_rename[i_rename12][j_rename12] = board_rename[j_rename12][(n_rename12 - i_rename12) - 1];
            board_rename[j_rename12][(n_rename12 - i_rename12) - 1] = board_rename[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1];
            board_rename[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1] = board_rename[(n_rename12 - j_rename12) - 1][i_rename12];
            board_rename[(n_rename12 - j_rename12) - 1][i_rename12] = tmp_rename12;
          }

        }

        return12:
        ;

      }
      {
        bool return_value;
        bool success_rename13 = false;
        uint8_t x_rename13;
        for (x_rename13 = 0; x_rename13 < 4; x_rename13++)
        {
          {
            bool return_value;
            bool success_rename25 = false;
            uint8_t x_rename25;
            uint8_t t_rename25;
            uint8_t stop_rename25 = 0;
            for (x_rename25 = 0; x_rename25 < 4; x_rename25++)
            {
              if (board_rename[x_rename13][x_rename25] != 0)
              {
                {
                  uint8_t return_value;
                  uint8_t stop_rename29 = stop_rename25;
                  uint8_t x_rename29 = x_rename25;
                  uint8_t t_rename29;
                  if (x_rename29 == 0)
                  {
                    loop_counter[34]++;
                    {
                      return_value = x_rename29;
                      goto return29;
                    }
                  }

                  for (t_rename29 = x_rename29 - 1; t_rename29 >= 0; t_rename29--)
                  {
                    loop_counter[35]++;
                    if (board_rename[x_rename13][t_rename29] != 0)
                    {
                      loop_counter[36]++;
                      if (board_rename[x_rename13][t_rename29] != board_rename[x_rename13][x_rename29])
                      {
                        {
                          return_value = t_rename29 + 1;
                          goto return29;
                        }
                      }

                      {
                        return_value = t_rename29;
                        goto return29;
                      }
                    }
                    else
                    {
                      if (t_rename29 == stop_rename29)
                      {
                        loop_counter[38]++;
                        {
                          return_value = t_rename29;
                          goto return29;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename29;
                    goto return29;
                  }
                  return29:
                  ;

                  t_rename25 = return_value;
                }
                if (t_rename25 != x_rename25)
                {
                  if (board_rename[x_rename13][t_rename25] == 0)
                  {
                    loop_counter[40]++;
                    board_rename[x_rename13][t_rename25] = board_rename[x_rename13][x_rename25];
                  }
                  else
                    if (board_rename[x_rename13][t_rename25] == board_rename[x_rename13][x_rename25])
                  {
                    loop_counter[41]++;
                    board_rename[x_rename13][t_rename25]++;
                    stop_rename25 = t_rename25 + 1;
                  }


                  board_rename[x_rename13][x_rename25] = 0;
                  success_rename25 = true;
                }

              }

            }

            {
              return_value = success_rename25;
              goto return25;
            }
            return25:
            ;

            success_rename13 = return_value;
          }
        }

        {
          return_value = success_rename13;
          goto return13;
        }
        return13:
        ;

        success_rename1 = return_value;
      }
      {
        uint8_t i_rename14;
        uint8_t j_rename14;
        uint8_t n_rename14 = 4;
        uint8_t tmp_rename14;
        for (i_rename14 = 0; i_rename14 < (n_rename14 / 2); i_rename14++)
        {
          for (j_rename14 = i_rename14; j_rename14 < ((n_rename14 - i_rename14) - 1); j_rename14++)
          {
            tmp_rename14 = board_rename[i_rename14][j_rename14];
            board_rename[i_rename14][j_rename14] = board_rename[j_rename14][(n_rename14 - i_rename14) - 1];
            board_rename[j_rename14][(n_rename14 - i_rename14) - 1] = board_rename[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1];
            board_rename[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1] = board_rename[(n_rename14 - j_rename14) - 1][i_rename14];
            board_rename[(n_rename14 - j_rename14) - 1][i_rename14] = tmp_rename14;
          }

        }

        return14:
        ;

      }
      {
        return_value = success_rename1;
        goto return1;
      }
      return1:
      ;

    }
      break;

    case 65:
      loop_counter[46]++;
    {
      bool return_value;
      bool success_rename2 = false;
      uint8_t x_rename2;
      for (x_rename2 = 0; x_rename2 < 4; x_rename2++)
      {
        {
          bool return_value;
          bool success_rename15 = false;
          uint8_t x_rename15;
          uint8_t t_rename15;
          uint8_t stop_rename15 = 0;
          for (x_rename15 = 0; x_rename15 < 4; x_rename15++)
          {
            if (board_rename[x_rename2][x_rename15] != 0)
            {
              {
                uint8_t return_value;
                uint8_t stop_rename26 = stop_rename15;
                uint8_t x_rename26 = x_rename15;
                uint8_t t_rename26;
                if (x_rename26 == 0)
                {
                  loop_counter[50]++;
                  {
                    return_value = x_rename26;
                    goto return26;
                  }
                }

                for (t_rename26 = x_rename26 - 1; t_rename26 >= 0; t_rename26--)
                {
                  loop_counter[51]++;
                  if (board_rename[x_rename2][t_rename26] != 0)
                  {
                    if (board_rename[x_rename2][t_rename26] != board_rename[x_rename2][x_rename26])
                    {
                      loop_counter[53]++;
                      {
                        return_value = t_rename26 + 1;
                        goto return26;
                      }
                    }

                    {
                      return_value = t_rename26;
                      goto return26;
                    }
                  }
                  else
                  {
                    if (t_rename26 == stop_rename26)
                    {
                      loop_counter[54]++;
                      {
                        return_value = t_rename26;
                        goto return26;
                      }
                    }

                  }

                }

                {
                  return_value = x_rename26;
                  goto return26;
                }
                return26:
                ;

                t_rename15 = return_value;
              }
              if (t_rename15 != x_rename15)
              {
                if (board_rename[x_rename2][t_rename15] == 0)
                {
                  loop_counter[56]++;
                  board_rename[x_rename2][t_rename15] = board_rename[x_rename2][x_rename15];
                }
                else
                  if (board_rename[x_rename2][t_rename15] == board_rename[x_rename2][x_rename15])
                {
                  loop_counter[57]++;
                  board_rename[x_rename2][t_rename15]++;
                  stop_rename15 = t_rename15 + 1;
                }


                board_rename[x_rename2][x_rename15] = 0;
                success_rename15 = true;
              }

            }

          }

          {
            return_value = success_rename15;
            goto return15;
          }
          return15:
          ;

          success_rename2 = return_value;
        }
      }

      {
        return_value = success_rename2;
        goto return2;
      }
      return2:
      ;

    }
      break;

    case 66:
      loop_counter[60]++;
    {
      bool return_value;
      bool success_rename3;
      {
        uint8_t i_rename16;
        uint8_t j_rename16;
        uint8_t n_rename16 = 4;
        uint8_t tmp_rename16;
        for (i_rename16 = 0; i_rename16 < (n_rename16 / 2); i_rename16++)
        {
          for (j_rename16 = i_rename16; j_rename16 < ((n_rename16 - i_rename16) - 1); j_rename16++)
          {
            tmp_rename16 = board_rename[i_rename16][j_rename16];
            board_rename[i_rename16][j_rename16] = board_rename[j_rename16][(n_rename16 - i_rename16) - 1];
            board_rename[j_rename16][(n_rename16 - i_rename16) - 1] = board_rename[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1];
            board_rename[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1] = board_rename[(n_rename16 - j_rename16) - 1][i_rename16];
            board_rename[(n_rename16 - j_rename16) - 1][i_rename16] = tmp_rename16;
          }

        }

        return16:
        ;

      }
      {
        uint8_t i_rename17;
        uint8_t j_rename17;
        uint8_t n_rename17 = 4;
        uint8_t tmp_rename17;
        for (i_rename17 = 0; i_rename17 < (n_rename17 / 2); i_rename17++)
        {
          for (j_rename17 = i_rename17; j_rename17 < ((n_rename17 - i_rename17) - 1); j_rename17++)
          {
            tmp_rename17 = board_rename[i_rename17][j_rename17];
            board_rename[i_rename17][j_rename17] = board_rename[j_rename17][(n_rename17 - i_rename17) - 1];
            board_rename[j_rename17][(n_rename17 - i_rename17) - 1] = board_rename[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1];
            board_rename[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1] = board_rename[(n_rename17 - j_rename17) - 1][i_rename17];
            board_rename[(n_rename17 - j_rename17) - 1][i_rename17] = tmp_rename17;
          }

        }

        return17:
        ;

      }
      {
        bool return_value;
        bool success_rename18 = false;
        uint8_t x_rename18;
        for (x_rename18 = 0; x_rename18 < 4; x_rename18++)
        {
          {
            bool return_value;
            bool success_rename27 = false;
            uint8_t x_rename27;
            uint8_t t_rename27;
            uint8_t stop_rename27 = 0;
            for (x_rename27 = 0; x_rename27 < 4; x_rename27++)
            {
              if (board_rename[x_rename18][x_rename27] != 0)
              {
                {
                  uint8_t return_value;
                  uint8_t stop_rename30 = stop_rename27;
                  uint8_t x_rename30 = x_rename27;
                  uint8_t t_rename30;
                  if (x_rename30 == 0)
                  {
                    loop_counter[68]++;
                    {
                      return_value = x_rename30;
                      goto return30;
                    }
                  }

                  for (t_rename30 = x_rename30 - 1; t_rename30 >= 0; t_rename30--)
                  {
                    loop_counter[69]++;
                    if (board_rename[x_rename18][t_rename30] != 0)
                    {
                      loop_counter[70]++;
                      if (board_rename[x_rename18][t_rename30] != board_rename[x_rename18][x_rename30])
                      {
                        {
                          return_value = t_rename30 + 1;
                          goto return30;
                        }
                      }

                      {
                        return_value = t_rename30;
                        goto return30;
                      }
                    }
                    else
                    {
                      if (t_rename30 == stop_rename30)
                      {
                        loop_counter[72]++;
                        {
                          return_value = t_rename30;
                          goto return30;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename30;
                    goto return30;
                  }
                  return30:
                  ;

                  t_rename27 = return_value;
                }
                if (t_rename27 != x_rename27)
                {
                  if (board_rename[x_rename18][t_rename27] == 0)
                  {
                    loop_counter[74]++;
                    board_rename[x_rename18][t_rename27] = board_rename[x_rename18][x_rename27];
                  }
                  else
                    if (board_rename[x_rename18][t_rename27] == board_rename[x_rename18][x_rename27])
                  {
                    loop_counter[75]++;
                    board_rename[x_rename18][t_rename27]++;
                    stop_rename27 = t_rename27 + 1;
                  }


                  board_rename[x_rename18][x_rename27] = 0;
                  success_rename27 = true;
                }

              }

            }

            {
              return_value = success_rename27;
              goto return27;
            }
            return27:
            ;

            success_rename18 = return_value;
          }
        }

        {
          return_value = success_rename18;
          goto return18;
        }
        return18:
        ;

        success_rename3 = return_value;
      }
      {
        uint8_t i_rename19;
        uint8_t j_rename19;
        uint8_t n_rename19 = 4;
        uint8_t tmp_rename19;
        for (i_rename19 = 0; i_rename19 < (n_rename19 / 2); i_rename19++)
        {
          for (j_rename19 = i_rename19; j_rename19 < ((n_rename19 - i_rename19) - 1); j_rename19++)
          {
            tmp_rename19 = board_rename[i_rename19][j_rename19];
            board_rename[i_rename19][j_rename19] = board_rename[j_rename19][(n_rename19 - i_rename19) - 1];
            board_rename[j_rename19][(n_rename19 - i_rename19) - 1] = board_rename[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1];
            board_rename[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1] = board_rename[(n_rename19 - j_rename19) - 1][i_rename19];
            board_rename[(n_rename19 - j_rename19) - 1][i_rename19] = tmp_rename19;
          }

        }

        return19:
        ;

      }
      {
        uint8_t i_rename20;
        uint8_t j_rename20;
        uint8_t n_rename20 = 4;
        uint8_t tmp_rename20;
        for (i_rename20 = 0; i_rename20 < (n_rename20 / 2); i_rename20++)
        {
          for (j_rename20 = i_rename20; j_rename20 < ((n_rename20 - i_rename20) - 1); j_rename20++)
          {
            tmp_rename20 = board_rename[i_rename20][j_rename20];
            board_rename[i_rename20][j_rename20] = board_rename[j_rename20][(n_rename20 - i_rename20) - 1];
            board_rename[j_rename20][(n_rename20 - i_rename20) - 1] = board_rename[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1];
            board_rename[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1] = board_rename[(n_rename20 - j_rename20) - 1][i_rename20];
            board_rename[(n_rename20 - j_rename20) - 1][i_rename20] = tmp_rename20;
          }

        }

        return20:
        ;

      }
      {
        return_value = success_rename3;
        goto return3;
      }
      return3:
      ;

    }
      break;

  }

  {
    uint8_t x_rename4;
    uint8_t y_rename4;
    for (y_rename4 = 0; y_rename4 < 4; y_rename4++)
    {
      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        {
          uint8_t value_rename21 = board_rename[x_rename4][y_rename4];
          if (value_rename21 > 0)
          {
            while (value_rename21--)
            {
              loop_counter[84]++;
            }

          }

          return21:
          ;

        }
      }

      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        {
          return22:
          ;

        }
      }

      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        {
          return23:
          ;

        }
      }

    }

    return4:
    ;

  }
  {
    goto print_loop_counter;
  }
  {
    print_loop_counter:
    ;
#if GET_PREDICT || DEBUG_EN
    int i;
    printf("loop counter = (");
    for (i = 0; i < 95; i++) 
      printf("%d, ", loop_counter[i]);
    printf(")\n");
//write_array(loop_counter, 95);
#endif

  }
  {
    predict_exec_time:
    ;

    float exec_time;
    #if !CVX_EN //conservative
        exec_time = 0;
    #else //cvx    
        if(CVX_COEFF == 100)
            exec_time = 70.418501*loop_counter[2] + -14.042185*loop_counter[8] + 0.739281*loop_counter[9] + 7.637506*loop_counter[10] + -1.444099*loop_counter[12] + -0.258875*loop_counter[14] + -14.996773*loop_counter[15] + 133.148516*loop_counter[24] + 16.557987*loop_counter[34] + 4.153631*loop_counter[35] + -11.733714*loop_counter[36] + 4.872199*loop_counter[38] + -6.532572*loop_counter[40] + -13.024585*loop_counter[41] + 134.114372*loop_counter[46] + -27.634695*loop_counter[50] + -0.825882*loop_counter[51] + 8.421979*loop_counter[53] + -20.420673*loop_counter[54] + 8.740144*loop_counter[56] + 9.999654*loop_counter[57] + 127.309234*loop_counter[60] + -4.558783*loop_counter[68] + 0.868603*loop_counter[69] + -4.290456*loop_counter[70] + -17.330567*loop_counter[72] + 5.221300*loop_counter[74] + 11.944675*loop_counter[75] + 0.709544*loop_counter[84] + 370.941112;

    #endif
    return exec_time;
  }
}
#else // LITTLE
float main_loop_slice_reduced(char c, uint8_t board[4][4])
{
  uint8_t board_rename[4][4];
  int board_i0;
  for (board_i0 = 0; board_i0 < 4; board_i0++)
  {
    int board_i1;
    for (board_i1 = 0; board_i1 < 4; board_i1++)
    {
      board_rename[board_i0][board_i1] = board[board_i0][board_i1];
    }

  }

  int loop_counter[95] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  switch (c)
  {
    case 68:
      loop_counter[2]++;
    {
      bool return_value;
      bool success_rename0;
      {
        uint8_t i_rename5;
        uint8_t j_rename5;
        uint8_t n_rename5 = 4;
        uint8_t tmp_rename5;
        for (i_rename5 = 0; i_rename5 < (n_rename5 / 2); i_rename5++)
        {
          for (j_rename5 = i_rename5; j_rename5 < ((n_rename5 - i_rename5) - 1); j_rename5++)
          {
            tmp_rename5 = board_rename[i_rename5][j_rename5];
            board_rename[i_rename5][j_rename5] = board_rename[j_rename5][(n_rename5 - i_rename5) - 1];
            board_rename[j_rename5][(n_rename5 - i_rename5) - 1] = board_rename[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1];
            board_rename[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1] = board_rename[(n_rename5 - j_rename5) - 1][i_rename5];
            board_rename[(n_rename5 - j_rename5) - 1][i_rename5] = tmp_rename5;
          }

        }

        return5:
        ;

      }
      {
        bool return_value;
        bool success_rename6 = false;
        uint8_t x_rename6;
        for (x_rename6 = 0; x_rename6 < 4; x_rename6++)
        {
          {
            bool return_value;
            bool success_rename24 = false;
            uint8_t x_rename24;
            uint8_t t_rename24;
            uint8_t stop_rename24 = 0;
            for (x_rename24 = 0; x_rename24 < 4; x_rename24++)
            {
              if (board_rename[x_rename6][x_rename24] != 0)
              {
                {
                  uint8_t return_value;
                  uint8_t stop_rename28 = stop_rename24;
                  uint8_t x_rename28 = x_rename24;
                  uint8_t t_rename28;
                  if (x_rename28 == 0)
                  {
                    loop_counter[8]++;
                    {
                      return_value = x_rename28;
                      goto return28;
                    }
                  }

                  for (t_rename28 = x_rename28 - 1; t_rename28 >= 0; t_rename28--)
                  {
                    loop_counter[9]++;
                    if (board_rename[x_rename6][t_rename28] != 0)
                    {
                      if (board_rename[x_rename6][t_rename28] != board_rename[x_rename6][x_rename28])
                      {
                        loop_counter[11]++;
                        {
                          return_value = t_rename28 + 1;
                          goto return28;
                        }
                      }

                      {
                        return_value = t_rename28;
                        goto return28;
                      }
                    }
                    else
                    {
                      if (t_rename28 == stop_rename28)
                      {
                        loop_counter[12]++;
                        {
                          return_value = t_rename28;
                          goto return28;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename28;
                    goto return28;
                  }
                  return28:
                  ;

                  t_rename24 = return_value;
                }
                if (t_rename24 != x_rename24)
                {
                  if (board_rename[x_rename6][t_rename24] == 0)
                  {
                    loop_counter[14]++;
                    board_rename[x_rename6][t_rename24] = board_rename[x_rename6][x_rename24];
                  }
                  else
                    if (board_rename[x_rename6][t_rename24] == board_rename[x_rename6][x_rename24])
                  {
                    loop_counter[15]++;
                    board_rename[x_rename6][t_rename24]++;
                    stop_rename24 = t_rename24 + 1;
                  }


                  board_rename[x_rename6][x_rename24] = 0;
                  success_rename24 = true;
                }

              }

            }

            {
              return_value = success_rename24;
              goto return24;
            }
            return24:
            ;

            success_rename6 = return_value;
          }
        }

        {
          return_value = success_rename6;
          goto return6;
        }
        return6:
        ;

        success_rename0 = return_value;
      }
      {
        uint8_t i_rename7;
        uint8_t j_rename7;
        uint8_t n_rename7 = 4;
        uint8_t tmp_rename7;
        for (i_rename7 = 0; i_rename7 < (n_rename7 / 2); i_rename7++)
        {
          for (j_rename7 = i_rename7; j_rename7 < ((n_rename7 - i_rename7) - 1); j_rename7++)
          {
            tmp_rename7 = board_rename[i_rename7][j_rename7];
            board_rename[i_rename7][j_rename7] = board_rename[j_rename7][(n_rename7 - i_rename7) - 1];
            board_rename[j_rename7][(n_rename7 - i_rename7) - 1] = board_rename[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1];
            board_rename[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1] = board_rename[(n_rename7 - j_rename7) - 1][i_rename7];
            board_rename[(n_rename7 - j_rename7) - 1][i_rename7] = tmp_rename7;
          }

        }

        return7:
        ;

      }
      {
        uint8_t i_rename8;
        uint8_t j_rename8;
        uint8_t n_rename8 = 4;
        uint8_t tmp_rename8;
        for (i_rename8 = 0; i_rename8 < (n_rename8 / 2); i_rename8++)
        {
          for (j_rename8 = i_rename8; j_rename8 < ((n_rename8 - i_rename8) - 1); j_rename8++)
          {
            tmp_rename8 = board_rename[i_rename8][j_rename8];
            board_rename[i_rename8][j_rename8] = board_rename[j_rename8][(n_rename8 - i_rename8) - 1];
            board_rename[j_rename8][(n_rename8 - i_rename8) - 1] = board_rename[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1];
            board_rename[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1] = board_rename[(n_rename8 - j_rename8) - 1][i_rename8];
            board_rename[(n_rename8 - j_rename8) - 1][i_rename8] = tmp_rename8;
          }

        }

        return8:
        ;

      }
      {
        uint8_t i_rename9;
        uint8_t j_rename9;
        uint8_t n_rename9 = 4;
        uint8_t tmp_rename9;
        for (i_rename9 = 0; i_rename9 < (n_rename9 / 2); i_rename9++)
        {
          for (j_rename9 = i_rename9; j_rename9 < ((n_rename9 - i_rename9) - 1); j_rename9++)
          {
            tmp_rename9 = board_rename[i_rename9][j_rename9];
            board_rename[i_rename9][j_rename9] = board_rename[j_rename9][(n_rename9 - i_rename9) - 1];
            board_rename[j_rename9][(n_rename9 - i_rename9) - 1] = board_rename[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1];
            board_rename[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1] = board_rename[(n_rename9 - j_rename9) - 1][i_rename9];
            board_rename[(n_rename9 - j_rename9) - 1][i_rename9] = tmp_rename9;
          }

        }

        return9:
        ;

      }
      {
        return_value = success_rename0;
        goto return0;
      }
      return0:
      ;

    }
      break;

    case 67:
      loop_counter[24]++;
    {
      bool return_value;
      bool success_rename1;
      {
        uint8_t i_rename10;
        uint8_t j_rename10;
        uint8_t n_rename10 = 4;
        uint8_t tmp_rename10;
        for (i_rename10 = 0; i_rename10 < (n_rename10 / 2); i_rename10++)
        {
          for (j_rename10 = i_rename10; j_rename10 < ((n_rename10 - i_rename10) - 1); j_rename10++)
          {
            tmp_rename10 = board_rename[i_rename10][j_rename10];
            board_rename[i_rename10][j_rename10] = board_rename[j_rename10][(n_rename10 - i_rename10) - 1];
            board_rename[j_rename10][(n_rename10 - i_rename10) - 1] = board_rename[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1];
            board_rename[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1] = board_rename[(n_rename10 - j_rename10) - 1][i_rename10];
            board_rename[(n_rename10 - j_rename10) - 1][i_rename10] = tmp_rename10;
          }

        }

        return10:
        ;

      }
      {
        uint8_t i_rename11;
        uint8_t j_rename11;
        uint8_t n_rename11 = 4;
        uint8_t tmp_rename11;
        for (i_rename11 = 0; i_rename11 < (n_rename11 / 2); i_rename11++)
        {
          for (j_rename11 = i_rename11; j_rename11 < ((n_rename11 - i_rename11) - 1); j_rename11++)
          {
            tmp_rename11 = board_rename[i_rename11][j_rename11];
            board_rename[i_rename11][j_rename11] = board_rename[j_rename11][(n_rename11 - i_rename11) - 1];
            board_rename[j_rename11][(n_rename11 - i_rename11) - 1] = board_rename[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1];
            board_rename[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1] = board_rename[(n_rename11 - j_rename11) - 1][i_rename11];
            board_rename[(n_rename11 - j_rename11) - 1][i_rename11] = tmp_rename11;
          }

        }

        return11:
        ;

      }
      {
        uint8_t i_rename12;
        uint8_t j_rename12;
        uint8_t n_rename12 = 4;
        uint8_t tmp_rename12;
        for (i_rename12 = 0; i_rename12 < (n_rename12 / 2); i_rename12++)
        {
          for (j_rename12 = i_rename12; j_rename12 < ((n_rename12 - i_rename12) - 1); j_rename12++)
          {
            tmp_rename12 = board_rename[i_rename12][j_rename12];
            board_rename[i_rename12][j_rename12] = board_rename[j_rename12][(n_rename12 - i_rename12) - 1];
            board_rename[j_rename12][(n_rename12 - i_rename12) - 1] = board_rename[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1];
            board_rename[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1] = board_rename[(n_rename12 - j_rename12) - 1][i_rename12];
            board_rename[(n_rename12 - j_rename12) - 1][i_rename12] = tmp_rename12;
          }

        }

        return12:
        ;

      }
      {
        bool return_value;
        bool success_rename13 = false;
        uint8_t x_rename13;
        for (x_rename13 = 0; x_rename13 < 4; x_rename13++)
        {
          {
            bool return_value;
            bool success_rename25 = false;
            uint8_t x_rename25;
            uint8_t t_rename25;
            uint8_t stop_rename25 = 0;
            for (x_rename25 = 0; x_rename25 < 4; x_rename25++)
            {
              if (board_rename[x_rename13][x_rename25] != 0)
              {
                {
                  uint8_t return_value;
                  uint8_t stop_rename29 = stop_rename25;
                  uint8_t x_rename29 = x_rename25;
                  uint8_t t_rename29;
                  if (x_rename29 == 0)
                  {
                    loop_counter[34]++;
                    {
                      return_value = x_rename29;
                      goto return29;
                    }
                  }

                  for (t_rename29 = x_rename29 - 1; t_rename29 >= 0; t_rename29--)
                  {
                    loop_counter[35]++;
                    if (board_rename[x_rename13][t_rename29] != 0)
                    {
                      loop_counter[36]++;
                      if (board_rename[x_rename13][t_rename29] != board_rename[x_rename13][x_rename29])
                      {
                        {
                          return_value = t_rename29 + 1;
                          goto return29;
                        }
                      }

                      {
                        return_value = t_rename29;
                        goto return29;
                      }
                    }
                    else
                    {
                      if (t_rename29 == stop_rename29)
                      {
                        loop_counter[38]++;
                        {
                          return_value = t_rename29;
                          goto return29;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename29;
                    goto return29;
                  }
                  return29:
                  ;

                  t_rename25 = return_value;
                }
                if (t_rename25 != x_rename25)
                {
                  if (board_rename[x_rename13][t_rename25] == 0)
                  {
                    loop_counter[40]++;
                    board_rename[x_rename13][t_rename25] = board_rename[x_rename13][x_rename25];
                  }
                  else
                    if (board_rename[x_rename13][t_rename25] == board_rename[x_rename13][x_rename25])
                  {
                    loop_counter[41]++;
                    board_rename[x_rename13][t_rename25]++;
                    stop_rename25 = t_rename25 + 1;
                  }


                  board_rename[x_rename13][x_rename25] = 0;
                  success_rename25 = true;
                }

              }

            }

            {
              return_value = success_rename25;
              goto return25;
            }
            return25:
            ;

            success_rename13 = return_value;
          }
        }

        {
          return_value = success_rename13;
          goto return13;
        }
        return13:
        ;

        success_rename1 = return_value;
      }
      {
        uint8_t i_rename14;
        uint8_t j_rename14;
        uint8_t n_rename14 = 4;
        uint8_t tmp_rename14;
        for (i_rename14 = 0; i_rename14 < (n_rename14 / 2); i_rename14++)
        {
          for (j_rename14 = i_rename14; j_rename14 < ((n_rename14 - i_rename14) - 1); j_rename14++)
          {
            tmp_rename14 = board_rename[i_rename14][j_rename14];
            board_rename[i_rename14][j_rename14] = board_rename[j_rename14][(n_rename14 - i_rename14) - 1];
            board_rename[j_rename14][(n_rename14 - i_rename14) - 1] = board_rename[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1];
            board_rename[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1] = board_rename[(n_rename14 - j_rename14) - 1][i_rename14];
            board_rename[(n_rename14 - j_rename14) - 1][i_rename14] = tmp_rename14;
          }

        }

        return14:
        ;

      }
      {
        return_value = success_rename1;
        goto return1;
      }
      return1:
      ;

    }
      break;

    case 65:
      loop_counter[46]++;
    {
      bool return_value;
      bool success_rename2 = false;
      uint8_t x_rename2;
      for (x_rename2 = 0; x_rename2 < 4; x_rename2++)
      {
        {
          bool return_value;
          bool success_rename15 = false;
          uint8_t x_rename15;
          uint8_t t_rename15;
          uint8_t stop_rename15 = 0;
          for (x_rename15 = 0; x_rename15 < 4; x_rename15++)
          {
            if (board_rename[x_rename2][x_rename15] != 0)
            {
              {
                uint8_t return_value;
                uint8_t stop_rename26 = stop_rename15;
                uint8_t x_rename26 = x_rename15;
                uint8_t t_rename26;
                if (x_rename26 == 0)
                {
                  loop_counter[50]++;
                  {
                    return_value = x_rename26;
                    goto return26;
                  }
                }

                for (t_rename26 = x_rename26 - 1; t_rename26 >= 0; t_rename26--)
                {
                  loop_counter[51]++;
                  if (board_rename[x_rename2][t_rename26] != 0)
                  {
                    if (board_rename[x_rename2][t_rename26] != board_rename[x_rename2][x_rename26])
                    {
                      loop_counter[53]++;
                      {
                        return_value = t_rename26 + 1;
                        goto return26;
                      }
                    }

                    {
                      return_value = t_rename26;
                      goto return26;
                    }
                  }
                  else
                  {
                    if (t_rename26 == stop_rename26)
                    {
                      loop_counter[54]++;
                      {
                        return_value = t_rename26;
                        goto return26;
                      }
                    }

                  }

                }

                {
                  return_value = x_rename26;
                  goto return26;
                }
                return26:
                ;

                t_rename15 = return_value;
              }
              if (t_rename15 != x_rename15)
              {
                if (board_rename[x_rename2][t_rename15] == 0)
                {
                  loop_counter[56]++;
                  board_rename[x_rename2][t_rename15] = board_rename[x_rename2][x_rename15];
                }
                else
                  if (board_rename[x_rename2][t_rename15] == board_rename[x_rename2][x_rename15])
                {
                  board_rename[x_rename2][t_rename15]++;
                  stop_rename15 = t_rename15 + 1;
                }


                board_rename[x_rename2][x_rename15] = 0;
                success_rename15 = true;
              }

            }

          }

          {
            return_value = success_rename15;
            goto return15;
          }
          return15:
          ;

          success_rename2 = return_value;
        }
      }

      {
        return_value = success_rename2;
        goto return2;
      }
      return2:
      ;

    }
      break;

    case 66:
      loop_counter[60]++;
    {
      bool return_value;
      bool success_rename3;
      {
        uint8_t i_rename16;
        uint8_t j_rename16;
        uint8_t n_rename16 = 4;
        uint8_t tmp_rename16;
        for (i_rename16 = 0; i_rename16 < (n_rename16 / 2); i_rename16++)
        {
          for (j_rename16 = i_rename16; j_rename16 < ((n_rename16 - i_rename16) - 1); j_rename16++)
          {
            tmp_rename16 = board_rename[i_rename16][j_rename16];
            board_rename[i_rename16][j_rename16] = board_rename[j_rename16][(n_rename16 - i_rename16) - 1];
            board_rename[j_rename16][(n_rename16 - i_rename16) - 1] = board_rename[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1];
            board_rename[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1] = board_rename[(n_rename16 - j_rename16) - 1][i_rename16];
            board_rename[(n_rename16 - j_rename16) - 1][i_rename16] = tmp_rename16;
          }

        }

        return16:
        ;

      }
      {
        uint8_t i_rename17;
        uint8_t j_rename17;
        uint8_t n_rename17 = 4;
        uint8_t tmp_rename17;
        for (i_rename17 = 0; i_rename17 < (n_rename17 / 2); i_rename17++)
        {
          for (j_rename17 = i_rename17; j_rename17 < ((n_rename17 - i_rename17) - 1); j_rename17++)
          {
            tmp_rename17 = board_rename[i_rename17][j_rename17];
            board_rename[i_rename17][j_rename17] = board_rename[j_rename17][(n_rename17 - i_rename17) - 1];
            board_rename[j_rename17][(n_rename17 - i_rename17) - 1] = board_rename[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1];
            board_rename[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1] = board_rename[(n_rename17 - j_rename17) - 1][i_rename17];
            board_rename[(n_rename17 - j_rename17) - 1][i_rename17] = tmp_rename17;
          }

        }

        return17:
        ;

      }
      {
        bool return_value;
        bool success_rename18 = false;
        uint8_t x_rename18;
        for (x_rename18 = 0; x_rename18 < 4; x_rename18++)
        {
          {
            bool return_value;
            bool success_rename27 = false;
            uint8_t x_rename27;
            uint8_t t_rename27;
            uint8_t stop_rename27 = 0;
            for (x_rename27 = 0; x_rename27 < 4; x_rename27++)
            {
              if (board_rename[x_rename18][x_rename27] != 0)
              {
                {
                  uint8_t return_value;
                  uint8_t stop_rename30 = stop_rename27;
                  uint8_t x_rename30 = x_rename27;
                  uint8_t t_rename30;
                  if (x_rename30 == 0)
                  {
                    loop_counter[68]++;
                    {
                      return_value = x_rename30;
                      goto return30;
                    }
                  }

                  for (t_rename30 = x_rename30 - 1; t_rename30 >= 0; t_rename30--)
                  {
                    loop_counter[69]++;
                    if (board_rename[x_rename18][t_rename30] != 0)
                    {
                      if (board_rename[x_rename18][t_rename30] != board_rename[x_rename18][x_rename30])
                      {
                        loop_counter[71]++;
                        {
                          return_value = t_rename30 + 1;
                          goto return30;
                        }
                      }

                      {
                        return_value = t_rename30;
                        goto return30;
                      }
                    }
                    else
                    {
                      if (t_rename30 == stop_rename30)
                      {
                        {
                          return_value = t_rename30;
                          goto return30;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename30;
                    goto return30;
                  }
                  return30:
                  ;

                  t_rename27 = return_value;
                }
                if (t_rename27 != x_rename27)
                {
                  loop_counter[73]++;
                  if (board_rename[x_rename18][t_rename27] == 0)
                  {
                    board_rename[x_rename18][t_rename27] = board_rename[x_rename18][x_rename27];
                  }
                  else
                    if (board_rename[x_rename18][t_rename27] == board_rename[x_rename18][x_rename27])
                  {
                    loop_counter[75]++;
                    board_rename[x_rename18][t_rename27]++;
                    stop_rename27 = t_rename27 + 1;
                  }


                  board_rename[x_rename18][x_rename27] = 0;
                  success_rename27 = true;
                }

              }

            }

            {
              return_value = success_rename27;
              goto return27;
            }
            return27:
            ;

            success_rename18 = return_value;
          }
        }

        {
          return_value = success_rename18;
          goto return18;
        }
        return18:
        ;

        success_rename3 = return_value;
      }
      {
        uint8_t i_rename19;
        uint8_t j_rename19;
        uint8_t n_rename19 = 4;
        uint8_t tmp_rename19;
        for (i_rename19 = 0; i_rename19 < (n_rename19 / 2); i_rename19++)
        {
          for (j_rename19 = i_rename19; j_rename19 < ((n_rename19 - i_rename19) - 1); j_rename19++)
          {
            tmp_rename19 = board_rename[i_rename19][j_rename19];
            board_rename[i_rename19][j_rename19] = board_rename[j_rename19][(n_rename19 - i_rename19) - 1];
            board_rename[j_rename19][(n_rename19 - i_rename19) - 1] = board_rename[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1];
            board_rename[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1] = board_rename[(n_rename19 - j_rename19) - 1][i_rename19];
            board_rename[(n_rename19 - j_rename19) - 1][i_rename19] = tmp_rename19;
          }

        }

        return19:
        ;

      }
      {
        uint8_t i_rename20;
        uint8_t j_rename20;
        uint8_t n_rename20 = 4;
        uint8_t tmp_rename20;
        for (i_rename20 = 0; i_rename20 < (n_rename20 / 2); i_rename20++)
        {
          for (j_rename20 = i_rename20; j_rename20 < ((n_rename20 - i_rename20) - 1); j_rename20++)
          {
            tmp_rename20 = board_rename[i_rename20][j_rename20];
            board_rename[i_rename20][j_rename20] = board_rename[j_rename20][(n_rename20 - i_rename20) - 1];
            board_rename[j_rename20][(n_rename20 - i_rename20) - 1] = board_rename[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1];
            board_rename[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1] = board_rename[(n_rename20 - j_rename20) - 1][i_rename20];
            board_rename[(n_rename20 - j_rename20) - 1][i_rename20] = tmp_rename20;
          }

        }

        return20:
        ;

      }
      {
        return_value = success_rename3;
        goto return3;
      }
      return3:
      ;

    }
      break;

  }

  {
    uint8_t x_rename4;
    uint8_t y_rename4;
    for (y_rename4 = 0; y_rename4 < 4; y_rename4++)
    {
      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        {
          uint8_t value_rename21 = board_rename[x_rename4][y_rename4];
          if (value_rename21 > 0)
          {
            while (value_rename21--)
            {
              loop_counter[84]++;
            }

          }

          return21:
          ;

        }
      }

      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        {
          return22:
          ;

        }
      }

      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        {
          return23:
          ;

        }
      }

    }

    return4:
    ;

  }
  {
    goto print_loop_counter;
  }
  {
    print_loop_counter:
    ;
#if GET_PREDICT || DEBUG_EN
    int i;
    printf("loop counter = (");
    for (i = 0; i < 95; i++) 
      printf("%d, ", loop_counter[i]);
    printf(")\n");
    
//  write_array(loop_counter, 95);
#endif

  }
  {
    predict_exec_time:
    ;
    float exec_time;
#if !CVX_EN //conservative
    exec_time = 122.868000*loop_counter[2] + 261.677000*loop_counter[8] + -90.609600*loop_counter[9] + 15.047800*loop_counter[11] + 281.673000*loop_counter[12] + 58.613500*loop_counter[14] + 8.840640*loop_counter[15] + -215.377000*loop_counter[24] + 82.887500*loop_counter[34] + 23.840100*loop_counter[35] + -3.138030*loop_counter[36] + 38.234700*loop_counter[38] + -21.759300*loop_counter[40] + -73.804800*loop_counter[41] + -24.479300*loop_counter[50] + -7.347490*loop_counter[51] + 35.698500*loop_counter[53] + 79.723800*loop_counter[54] + -21.712800*loop_counter[56] + 124.679000*loop_counter[60] + 29.171300*loop_counter[68] + -1.146070*loop_counter[69] + 9.540680*loop_counter[71] + 3.880310*loop_counter[73] + -66.106100*loop_counter[75] + -3.597610*loop_counter[84] + 1909.640000;
#else //cvx
        if(CVX_COEFF == 10)
            exec_time = 560.162758*loop_counter[2] + 262.920307*loop_counter[8] + -90.086186*loop_counter[9] + 15.167749*loop_counter[11] + 283.134395*loop_counter[12] + 57.872098*loop_counter[14] + 6.616293*loop_counter[15] + 438.624604*loop_counter[24] + 2.901740*loop_counter[34] + 8.498630*loop_counter[35] + 13.123690*loop_counter[36] + 0.531196*loop_counter[38] + -13.761758*loop_counter[40] + -3.345834*loop_counter[41] + 444.466775*loop_counter[46] + -22.344746*loop_counter[50] + -9.731700*loop_counter[51] + 37.850944*loop_counter[53] + 82.025088*loop_counter[54] + -19.964876*loop_counter[56] + 550.028394*loop_counter[60] + 58.170062*loop_counter[68] + 0.658846*loop_counter[69] + -4.249692*loop_counter[71] + 5.185938*loop_counter[73] + -39.652290*loop_counter[75] + -3.728453*loop_counter[84] + 1466.328384;
        else if(CVX_COEFF == 50)
            exec_time = 615.379751*loop_counter[2] + 261.677291*loop_counter[8] + -90.609562*loop_counter[9] + 15.047809*loop_counter[11] + 281.673307*loop_counter[12] + 58.613546*loop_counter[14] + 8.840637*loop_counter[15] + 277.134402*loop_counter[24] + 82.887449*loop_counter[34] + 23.840139*loop_counter[35] + -3.138031*loop_counter[36] + 38.234728*loop_counter[38] + -21.759296*loop_counter[40] + -73.804781*loop_counter[41] + 492.511314*loop_counter[46] + -24.479251*loop_counter[50] + -7.347486*loop_counter[51] + 35.698472*loop_counter[53] + 79.723804*loop_counter[54] + -21.712839*loop_counter[56] + 617.190254*loop_counter[60] + 29.171315*loop_counter[68] + -1.146070*loop_counter[69] + 9.540681*loop_counter[71] + 3.880308*loop_counter[73] + -66.106078*loop_counter[75] + -3.597610*loop_counter[84] + 1417.130208;
        else if(CVX_COEFF == 100)
            exec_time = 615.379740*loop_counter[2] + 261.677291*loop_counter[8] + -90.609562*loop_counter[9] + 15.047809*loop_counter[11] + 281.673307*loop_counter[12] + 58.613546*loop_counter[14] + 8.840637*loop_counter[15] + 277.134390*loop_counter[24] + 82.887449*loop_counter[34] + 23.840139*loop_counter[35] + -3.138031*loop_counter[36] + 38.234728*loop_counter[38] + -21.759296*loop_counter[40] + -73.804781*loop_counter[41] + 492.511302*loop_counter[46] + -24.479251*loop_counter[50] + -7.347486*loop_counter[51] + 35.698472*loop_counter[53] + 79.723804*loop_counter[54] + -21.712839*loop_counter[56] + 617.190242*loop_counter[60] + 29.171315*loop_counter[68] + -1.146070*loop_counter[69] + 9.540681*loop_counter[71] + 3.880308*loop_counter[73] + -66.106078*loop_counter[75] + -3.597610*loop_counter[84] + 1417.130220;
#endif
    return exec_time;
  }
}
#endif
*/
bool main_loop_loop_counters(char c, uint8_t board[4][4])
{
  int loop_counter[94] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  bool success;
  switch (c)
  {
    case 97:
      loop_counter[0]++;

    case 104:
      loop_counter[1]++;

    case 68:
      loop_counter[2]++;
    {
      bool return_value;
      bool success_rename0;
      {
        uint8_t i_rename5;
        uint8_t j_rename5;
        uint8_t n_rename5 = 4;
        uint8_t tmp_rename5;
        for (i_rename5 = 0; i_rename5 < (n_rename5 / 2); i_rename5++)
        {
          loop_counter[3]++;
          for (j_rename5 = i_rename5; j_rename5 < ((n_rename5 - i_rename5) - 1); j_rename5++)
          {
            loop_counter[4]++;
            tmp_rename5 = board[i_rename5][j_rename5];
            board[i_rename5][j_rename5] = board[j_rename5][(n_rename5 - i_rename5) - 1];
            board[j_rename5][(n_rename5 - i_rename5) - 1] = board[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1];
            board[(n_rename5 - i_rename5) - 1][(n_rename5 - j_rename5) - 1] = board[(n_rename5 - j_rename5) - 1][i_rename5];
            board[(n_rename5 - j_rename5) - 1][i_rename5] = tmp_rename5;
          }

        }

        return5:
        ;

      }
      {
        bool return_value;
        bool success_rename6 = false;
        uint8_t x_rename6;
        for (x_rename6 = 0; x_rename6 < 4; x_rename6++)
        {
          loop_counter[5]++;
          {
            bool return_value;
            bool success_rename24 = false;
            uint8_t x_rename24;
            uint8_t t_rename24;
            uint8_t stop_rename24 = 0;
            for (x_rename24 = 0; x_rename24 < 4; x_rename24++)
            {
              loop_counter[6]++;
              if (board[x_rename6][x_rename24] != 0)
              {
                loop_counter[7]++;
                {
                  uint8_t return_value;
                  uint8_t stop_rename28 = stop_rename24;
                  uint8_t x_rename28 = x_rename24;
                  uint8_t t_rename28;
                  if (x_rename28 == 0)
                  {
                    loop_counter[8]++;
                    {
                      return_value = x_rename28;
                      goto return28;
                    }
                  }

                  for (t_rename28 = x_rename28 - 1; t_rename28 >= 0; t_rename28--)
                  {
                    loop_counter[9]++;
                    if (board[x_rename6][t_rename28] != 0)
                    {
                      loop_counter[10]++;
                      if (board[x_rename6][t_rename28] != board[x_rename6][x_rename28])
                      {
                        loop_counter[11]++;
                        {
                          return_value = t_rename28 + 1;
                          goto return28;
                        }
                      }

                      {
                        return_value = t_rename28;
                        goto return28;
                      }
                    }
                    else
                    {
                      if (t_rename28 == stop_rename28)
                      {
                        loop_counter[12]++;
                        {
                          return_value = t_rename28;
                          goto return28;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename28;
                    goto return28;
                  }
                  return28:
                  ;

                  t_rename24 = return_value;
                }
                if (t_rename24 != x_rename24)
                {
                  loop_counter[13]++;
                  if (board[x_rename6][t_rename24] == 0)
                  {
                    loop_counter[14]++;
                    board[x_rename6][t_rename24] = board[x_rename6][x_rename24];
                  }
                  else
                    if (board[x_rename6][t_rename24] == board[x_rename6][x_rename24])
                  {
                    loop_counter[15]++;
                    board[x_rename6][t_rename24]++;
                    score += ((uint32_t) 1) << board[x_rename6][t_rename24];
                    stop_rename24 = t_rename24 + 1;
                  }


                  board[x_rename6][x_rename24] = 0;
                  success_rename24 = true;
                }

              }

            }

            {
              return_value = success_rename24;
              goto return24;
            }
            return24:
            ;

            success_rename6 = return_value;
          }
        }

        {
          return_value = success_rename6;
          goto return6;
        }
        return6:
        ;

        success_rename0 = return_value;
      }
      {
        uint8_t i_rename7;
        uint8_t j_rename7;
        uint8_t n_rename7 = 4;
        uint8_t tmp_rename7;
        for (i_rename7 = 0; i_rename7 < (n_rename7 / 2); i_rename7++)
        {
          loop_counter[16]++;
          for (j_rename7 = i_rename7; j_rename7 < ((n_rename7 - i_rename7) - 1); j_rename7++)
          {
            loop_counter[17]++;
            tmp_rename7 = board[i_rename7][j_rename7];
            board[i_rename7][j_rename7] = board[j_rename7][(n_rename7 - i_rename7) - 1];
            board[j_rename7][(n_rename7 - i_rename7) - 1] = board[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1];
            board[(n_rename7 - i_rename7) - 1][(n_rename7 - j_rename7) - 1] = board[(n_rename7 - j_rename7) - 1][i_rename7];
            board[(n_rename7 - j_rename7) - 1][i_rename7] = tmp_rename7;
          }

        }

        return7:
        ;

      }
      {
        uint8_t i_rename8;
        uint8_t j_rename8;
        uint8_t n_rename8 = 4;
        uint8_t tmp_rename8;
        for (i_rename8 = 0; i_rename8 < (n_rename8 / 2); i_rename8++)
        {
          loop_counter[18]++;
          for (j_rename8 = i_rename8; j_rename8 < ((n_rename8 - i_rename8) - 1); j_rename8++)
          {
            loop_counter[19]++;
            tmp_rename8 = board[i_rename8][j_rename8];
            board[i_rename8][j_rename8] = board[j_rename8][(n_rename8 - i_rename8) - 1];
            board[j_rename8][(n_rename8 - i_rename8) - 1] = board[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1];
            board[(n_rename8 - i_rename8) - 1][(n_rename8 - j_rename8) - 1] = board[(n_rename8 - j_rename8) - 1][i_rename8];
            board[(n_rename8 - j_rename8) - 1][i_rename8] = tmp_rename8;
          }

        }

        return8:
        ;

      }
      {
        uint8_t i_rename9;
        uint8_t j_rename9;
        uint8_t n_rename9 = 4;
        uint8_t tmp_rename9;
        for (i_rename9 = 0; i_rename9 < (n_rename9 / 2); i_rename9++)
        {
          loop_counter[20]++;
          for (j_rename9 = i_rename9; j_rename9 < ((n_rename9 - i_rename9) - 1); j_rename9++)
          {
            loop_counter[21]++;
            tmp_rename9 = board[i_rename9][j_rename9];
            board[i_rename9][j_rename9] = board[j_rename9][(n_rename9 - i_rename9) - 1];
            board[j_rename9][(n_rename9 - i_rename9) - 1] = board[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1];
            board[(n_rename9 - i_rename9) - 1][(n_rename9 - j_rename9) - 1] = board[(n_rename9 - j_rename9) - 1][i_rename9];
            board[(n_rename9 - j_rename9) - 1][i_rename9] = tmp_rename9;
          }

        }

        return9:
        ;

      }
      {
        return_value = success_rename0;
        goto return0;
      }
      return0:
      ;

      success = return_value;
    }
      break;

    case 100:
      loop_counter[22]++;

    case 108:
      loop_counter[23]++;

    case 67:
      loop_counter[24]++;
    {
      bool return_value;
      bool success_rename1;
      {
        uint8_t i_rename10;
        uint8_t j_rename10;
        uint8_t n_rename10 = 4;
        uint8_t tmp_rename10;
        for (i_rename10 = 0; i_rename10 < (n_rename10 / 2); i_rename10++)
        {
          loop_counter[25]++;
          for (j_rename10 = i_rename10; j_rename10 < ((n_rename10 - i_rename10) - 1); j_rename10++)
          {
            loop_counter[26]++;
            tmp_rename10 = board[i_rename10][j_rename10];
            board[i_rename10][j_rename10] = board[j_rename10][(n_rename10 - i_rename10) - 1];
            board[j_rename10][(n_rename10 - i_rename10) - 1] = board[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1];
            board[(n_rename10 - i_rename10) - 1][(n_rename10 - j_rename10) - 1] = board[(n_rename10 - j_rename10) - 1][i_rename10];
            board[(n_rename10 - j_rename10) - 1][i_rename10] = tmp_rename10;
          }

        }

        return10:
        ;

      }
      {
        uint8_t i_rename11;
        uint8_t j_rename11;
        uint8_t n_rename11 = 4;
        uint8_t tmp_rename11;
        for (i_rename11 = 0; i_rename11 < (n_rename11 / 2); i_rename11++)
        {
          loop_counter[27]++;
          for (j_rename11 = i_rename11; j_rename11 < ((n_rename11 - i_rename11) - 1); j_rename11++)
          {
            loop_counter[28]++;
            tmp_rename11 = board[i_rename11][j_rename11];
            board[i_rename11][j_rename11] = board[j_rename11][(n_rename11 - i_rename11) - 1];
            board[j_rename11][(n_rename11 - i_rename11) - 1] = board[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1];
            board[(n_rename11 - i_rename11) - 1][(n_rename11 - j_rename11) - 1] = board[(n_rename11 - j_rename11) - 1][i_rename11];
            board[(n_rename11 - j_rename11) - 1][i_rename11] = tmp_rename11;
          }

        }

        return11:
        ;

      }
      {
        uint8_t i_rename12;
        uint8_t j_rename12;
        uint8_t n_rename12 = 4;
        uint8_t tmp_rename12;
        for (i_rename12 = 0; i_rename12 < (n_rename12 / 2); i_rename12++)
        {
          loop_counter[29]++;
          for (j_rename12 = i_rename12; j_rename12 < ((n_rename12 - i_rename12) - 1); j_rename12++)
          {
            loop_counter[30]++;
            tmp_rename12 = board[i_rename12][j_rename12];
            board[i_rename12][j_rename12] = board[j_rename12][(n_rename12 - i_rename12) - 1];
            board[j_rename12][(n_rename12 - i_rename12) - 1] = board[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1];
            board[(n_rename12 - i_rename12) - 1][(n_rename12 - j_rename12) - 1] = board[(n_rename12 - j_rename12) - 1][i_rename12];
            board[(n_rename12 - j_rename12) - 1][i_rename12] = tmp_rename12;
          }

        }

        return12:
        ;

      }
      {
        bool return_value;
        bool success_rename13 = false;
        uint8_t x_rename13;
        for (x_rename13 = 0; x_rename13 < 4; x_rename13++)
        {
          loop_counter[31]++;
          {
            bool return_value;
            bool success_rename25 = false;
            uint8_t x_rename25;
            uint8_t t_rename25;
            uint8_t stop_rename25 = 0;
            for (x_rename25 = 0; x_rename25 < 4; x_rename25++)
            {
              loop_counter[32]++;
              if (board[x_rename13][x_rename25] != 0)
              {
                loop_counter[33]++;
                {
                  uint8_t return_value;
                  uint8_t stop_rename29 = stop_rename25;
                  uint8_t x_rename29 = x_rename25;
                  uint8_t t_rename29;
                  if (x_rename29 == 0)
                  {
                    loop_counter[34]++;
                    {
                      return_value = x_rename29;
                      goto return29;
                    }
                  }

                  for (t_rename29 = x_rename29 - 1; t_rename29 >= 0; t_rename29--)
                  {
                    loop_counter[35]++;
                    if (board[x_rename13][t_rename29] != 0)
                    {
                      loop_counter[36]++;
                      if (board[x_rename13][t_rename29] != board[x_rename13][x_rename29])
                      {
                        loop_counter[37]++;
                        {
                          return_value = t_rename29 + 1;
                          goto return29;
                        }
                      }

                      {
                        return_value = t_rename29;
                        goto return29;
                      }
                    }
                    else
                    {
                      if (t_rename29 == stop_rename29)
                      {
                        loop_counter[38]++;
                        {
                          return_value = t_rename29;
                          goto return29;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename29;
                    goto return29;
                  }
                  return29:
                  ;

                  t_rename25 = return_value;
                }
                if (t_rename25 != x_rename25)
                {
                  loop_counter[39]++;
                  if (board[x_rename13][t_rename25] == 0)
                  {
                    loop_counter[40]++;
                    board[x_rename13][t_rename25] = board[x_rename13][x_rename25];
                  }
                  else
                    if (board[x_rename13][t_rename25] == board[x_rename13][x_rename25])
                  {
                    loop_counter[41]++;
                    board[x_rename13][t_rename25]++;
                    score += ((uint32_t) 1) << board[x_rename13][t_rename25];
                    stop_rename25 = t_rename25 + 1;
                  }


                  board[x_rename13][x_rename25] = 0;
                  success_rename25 = true;
                }

              }

            }

            {
              return_value = success_rename25;
              goto return25;
            }
            return25:
            ;

            success_rename13 = return_value;
          }
        }

        {
          return_value = success_rename13;
          goto return13;
        }
        return13:
        ;

        success_rename1 = return_value;
      }
      {
        uint8_t i_rename14;
        uint8_t j_rename14;
        uint8_t n_rename14 = 4;
        uint8_t tmp_rename14;
        for (i_rename14 = 0; i_rename14 < (n_rename14 / 2); i_rename14++)
        {
          loop_counter[42]++;
          for (j_rename14 = i_rename14; j_rename14 < ((n_rename14 - i_rename14) - 1); j_rename14++)
          {
            loop_counter[43]++;
            tmp_rename14 = board[i_rename14][j_rename14];
            board[i_rename14][j_rename14] = board[j_rename14][(n_rename14 - i_rename14) - 1];
            board[j_rename14][(n_rename14 - i_rename14) - 1] = board[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1];
            board[(n_rename14 - i_rename14) - 1][(n_rename14 - j_rename14) - 1] = board[(n_rename14 - j_rename14) - 1][i_rename14];
            board[(n_rename14 - j_rename14) - 1][i_rename14] = tmp_rename14;
          }

        }

        return14:
        ;

      }
      {
        return_value = success_rename1;
        goto return1;
      }
      return1:
      ;

      success = return_value;
    }
      break;

    case 119:
      loop_counter[44]++;

    case 107:
      loop_counter[45]++;

    case 65:
      loop_counter[46]++;
    {
      bool return_value;
      bool success_rename2 = false;
      uint8_t x_rename2;
      for (x_rename2 = 0; x_rename2 < 4; x_rename2++)
      {
        loop_counter[47]++;
        {
          bool return_value;
          bool success_rename15 = false;
          uint8_t x_rename15;
          uint8_t t_rename15;
          uint8_t stop_rename15 = 0;
          for (x_rename15 = 0; x_rename15 < 4; x_rename15++)
          {
            loop_counter[48]++;
            if (board[x_rename2][x_rename15] != 0)
            {
              loop_counter[49]++;
              {
                uint8_t return_value;
                uint8_t stop_rename26 = stop_rename15;
                uint8_t x_rename26 = x_rename15;
                uint8_t t_rename26;
                if (x_rename26 == 0)
                {
                  loop_counter[50]++;
                  {
                    return_value = x_rename26;
                    goto return26;
                  }
                }

                for (t_rename26 = x_rename26 - 1; t_rename26 >= 0; t_rename26--)
                {
                  loop_counter[51]++;
                  if (board[x_rename2][t_rename26] != 0)
                  {
                    loop_counter[52]++;
                    if (board[x_rename2][t_rename26] != board[x_rename2][x_rename26])
                    {
                      loop_counter[53]++;
                      {
                        return_value = t_rename26 + 1;
                        goto return26;
                      }
                    }

                    {
                      return_value = t_rename26;
                      goto return26;
                    }
                  }
                  else
                  {
                    if (t_rename26 == stop_rename26)
                    {
                      loop_counter[54]++;
                      {
                        return_value = t_rename26;
                        goto return26;
                      }
                    }

                  }

                }

                {
                  return_value = x_rename26;
                  goto return26;
                }
                return26:
                ;

                t_rename15 = return_value;
              }
              if (t_rename15 != x_rename15)
              {
                loop_counter[55]++;
                if (board[x_rename2][t_rename15] == 0)
                {
                  loop_counter[56]++;
                  board[x_rename2][t_rename15] = board[x_rename2][x_rename15];
                }
                else
                  if (board[x_rename2][t_rename15] == board[x_rename2][x_rename15])
                {
                  loop_counter[57]++;
                  board[x_rename2][t_rename15]++;
                  score += ((uint32_t) 1) << board[x_rename2][t_rename15];
                  stop_rename15 = t_rename15 + 1;
                }


                board[x_rename2][x_rename15] = 0;
                success_rename15 = true;
              }

            }

          }

          {
            return_value = success_rename15;
            goto return15;
          }
          return15:
          ;

          success_rename2 = return_value;
        }
      }

      {
        return_value = success_rename2;
        goto return2;
      }
      return2:
      ;

      success = return_value;
    }
      break;

    case 115:
      loop_counter[58]++;

    case 106:
      loop_counter[59]++;

    case 66:
      loop_counter[60]++;
    {
      bool return_value;
      bool success_rename3;
      {
        uint8_t i_rename16;
        uint8_t j_rename16;
        uint8_t n_rename16 = 4;
        uint8_t tmp_rename16;
        for (i_rename16 = 0; i_rename16 < (n_rename16 / 2); i_rename16++)
        {
          loop_counter[61]++;
          for (j_rename16 = i_rename16; j_rename16 < ((n_rename16 - i_rename16) - 1); j_rename16++)
          {
            loop_counter[62]++;
            tmp_rename16 = board[i_rename16][j_rename16];
            board[i_rename16][j_rename16] = board[j_rename16][(n_rename16 - i_rename16) - 1];
            board[j_rename16][(n_rename16 - i_rename16) - 1] = board[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1];
            board[(n_rename16 - i_rename16) - 1][(n_rename16 - j_rename16) - 1] = board[(n_rename16 - j_rename16) - 1][i_rename16];
            board[(n_rename16 - j_rename16) - 1][i_rename16] = tmp_rename16;
          }

        }

        return16:
        ;

      }
      {
        uint8_t i_rename17;
        uint8_t j_rename17;
        uint8_t n_rename17 = 4;
        uint8_t tmp_rename17;
        for (i_rename17 = 0; i_rename17 < (n_rename17 / 2); i_rename17++)
        {
          loop_counter[63]++;
          for (j_rename17 = i_rename17; j_rename17 < ((n_rename17 - i_rename17) - 1); j_rename17++)
          {
            loop_counter[64]++;
            tmp_rename17 = board[i_rename17][j_rename17];
            board[i_rename17][j_rename17] = board[j_rename17][(n_rename17 - i_rename17) - 1];
            board[j_rename17][(n_rename17 - i_rename17) - 1] = board[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1];
            board[(n_rename17 - i_rename17) - 1][(n_rename17 - j_rename17) - 1] = board[(n_rename17 - j_rename17) - 1][i_rename17];
            board[(n_rename17 - j_rename17) - 1][i_rename17] = tmp_rename17;
          }

        }

        return17:
        ;

      }
      {
        bool return_value;
        bool success_rename18 = false;
        uint8_t x_rename18;
        for (x_rename18 = 0; x_rename18 < 4; x_rename18++)
        {
          loop_counter[65]++;
          {
            bool return_value;
            bool success_rename27 = false;
            uint8_t x_rename27;
            uint8_t t_rename27;
            uint8_t stop_rename27 = 0;
            for (x_rename27 = 0; x_rename27 < 4; x_rename27++)
            {
              loop_counter[66]++;
              if (board[x_rename18][x_rename27] != 0)
              {
                loop_counter[67]++;
                {
                  uint8_t return_value;
                  uint8_t stop_rename30 = stop_rename27;
                  uint8_t x_rename30 = x_rename27;
                  uint8_t t_rename30;
                  if (x_rename30 == 0)
                  {
                    loop_counter[68]++;
                    {
                      return_value = x_rename30;
                      goto return30;
                    }
                  }

                  for (t_rename30 = x_rename30 - 1; t_rename30 >= 0; t_rename30--)
                  {
                    loop_counter[69]++;
                    if (board[x_rename18][t_rename30] != 0)
                    {
                      loop_counter[70]++;
                      if (board[x_rename18][t_rename30] != board[x_rename18][x_rename30])
                      {
                        loop_counter[71]++;
                        {
                          return_value = t_rename30 + 1;
                          goto return30;
                        }
                      }

                      {
                        return_value = t_rename30;
                        goto return30;
                      }
                    }
                    else
                    {
                      if (t_rename30 == stop_rename30)
                      {
                        loop_counter[72]++;
                        {
                          return_value = t_rename30;
                          goto return30;
                        }
                      }

                    }

                  }

                  {
                    return_value = x_rename30;
                    goto return30;
                  }
                  return30:
                  ;

                  t_rename27 = return_value;
                }
                if (t_rename27 != x_rename27)
                {
                  loop_counter[73]++;
                  if (board[x_rename18][t_rename27] == 0)
                  {
                    loop_counter[74]++;
                    board[x_rename18][t_rename27] = board[x_rename18][x_rename27];
                  }
                  else
                    if (board[x_rename18][t_rename27] == board[x_rename18][x_rename27])
                  {
                    loop_counter[75]++;
                    board[x_rename18][t_rename27]++;
                    score += ((uint32_t) 1) << board[x_rename18][t_rename27];
                    stop_rename27 = t_rename27 + 1;
                  }


                  board[x_rename18][x_rename27] = 0;
                  success_rename27 = true;
                }

              }

            }

            {
              return_value = success_rename27;
              goto return27;
            }
            return27:
            ;

            success_rename18 = return_value;
          }
        }

        {
          return_value = success_rename18;
          goto return18;
        }
        return18:
        ;

        success_rename3 = return_value;
      }
      {
        uint8_t i_rename19;
        uint8_t j_rename19;
        uint8_t n_rename19 = 4;
        uint8_t tmp_rename19;
        for (i_rename19 = 0; i_rename19 < (n_rename19 / 2); i_rename19++)
        {
          loop_counter[76]++;
          for (j_rename19 = i_rename19; j_rename19 < ((n_rename19 - i_rename19) - 1); j_rename19++)
          {
            loop_counter[77]++;
            tmp_rename19 = board[i_rename19][j_rename19];
            board[i_rename19][j_rename19] = board[j_rename19][(n_rename19 - i_rename19) - 1];
            board[j_rename19][(n_rename19 - i_rename19) - 1] = board[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1];
            board[(n_rename19 - i_rename19) - 1][(n_rename19 - j_rename19) - 1] = board[(n_rename19 - j_rename19) - 1][i_rename19];
            board[(n_rename19 - j_rename19) - 1][i_rename19] = tmp_rename19;
          }

        }

        return19:
        ;

      }
      {
        uint8_t i_rename20;
        uint8_t j_rename20;
        uint8_t n_rename20 = 4;
        uint8_t tmp_rename20;
        for (i_rename20 = 0; i_rename20 < (n_rename20 / 2); i_rename20++)
        {
          loop_counter[78]++;
          for (j_rename20 = i_rename20; j_rename20 < ((n_rename20 - i_rename20) - 1); j_rename20++)
          {
            loop_counter[79]++;
            tmp_rename20 = board[i_rename20][j_rename20];
            board[i_rename20][j_rename20] = board[j_rename20][(n_rename20 - i_rename20) - 1];
            board[j_rename20][(n_rename20 - i_rename20) - 1] = board[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1];
            board[(n_rename20 - i_rename20) - 1][(n_rename20 - j_rename20) - 1] = board[(n_rename20 - j_rename20) - 1][i_rename20];
            board[(n_rename20 - j_rename20) - 1][i_rename20] = tmp_rename20;
          }

        }

        return20:
        ;

      }
      {
        return_value = success_rename3;
        goto return3;
      }
      return3:
      ;

      success = return_value;
    }
      break;

    default:
      success = false;

  }

  {
    uint8_t x_rename4;
    uint8_t y_rename4;
    char c_rename4;
    char color_rename4[40];
    char reset_rename4[] = "\e[m";
    printf("\e[H");
    printf("2048.c %17d pts\n", score);
    for (y_rename4 = 0; y_rename4 < 4; y_rename4++)
    {
      loop_counter[80]++;
      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        loop_counter[81]++;
        {
          size_t length_rename21 = 40;
          uint8_t value_rename21 = board[x_rename4][y_rename4];
          uint8_t original_rename21[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
          uint8_t blackwhite_rename21[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
          uint8_t bluered_rename21[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
          uint8_t *schemes_rename21[] = {original_rename21, blackwhite_rename21, bluered_rename21};
          uint8_t *background_rename21 = schemes_rename21[scheme] + 0;
          uint8_t *foreground_rename21 = schemes_rename21[scheme] + 1;
          if (value_rename21 > 0)
          {
            loop_counter[82]++;
            while (value_rename21--)
            {
              loop_counter[83]++;
              if ((background_rename21 + 2) < (schemes_rename21[scheme] + (sizeof(original_rename21))))
              {
                loop_counter[84]++;
                background_rename21 += 2;
                foreground_rename21 += 2;
              }

            }

          }

          snprintf(color_rename4, length_rename21, "\e[38;5;%d;48;5;%dm", *foreground_rename21, *background_rename21);
          return21:
          ;

        }
        printf("%s", color_rename4);
        printf("       ");
        printf("%s", reset_rename4);
      }

      printf("\n");
      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        loop_counter[85]++;
        {
          size_t length_rename22 = 40;
          uint8_t value_rename22 = board[x_rename4][y_rename4];
          uint8_t original_rename22[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
          uint8_t blackwhite_rename22[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
          uint8_t bluered_rename22[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
          uint8_t *schemes_rename22[] = {original_rename22, blackwhite_rename22, bluered_rename22};
          uint8_t *background_rename22 = schemes_rename22[scheme] + 0;
          uint8_t *foreground_rename22 = schemes_rename22[scheme] + 1;
          if (value_rename22 > 0)
          {
            loop_counter[86]++;
            while (value_rename22--)
            {
              loop_counter[87]++;
              if ((background_rename22 + 2) < (schemes_rename22[scheme] + (sizeof(original_rename22))))
              {
                loop_counter[88]++;
                background_rename22 += 2;
                foreground_rename22 += 2;
              }

            }

          }

          snprintf(color_rename4, length_rename22, "\e[38;5;%d;48;5;%dm", *foreground_rename22, *background_rename22);
          return22:
          ;

        }
        printf("%s", color_rename4);
        if (board[x_rename4][y_rename4] != 0)
        {
          loop_counter[89]++;
          char s_rename4[8];
          snprintf(s_rename4, 8, "%u", ((uint32_t) 1) << board[x_rename4][y_rename4]);
          uint8_t t_rename4 = 7 - strlen(s_rename4);
          printf("%*s%s%*s", t_rename4 - (t_rename4 / 2), "", s_rename4, t_rename4 / 2, "");
        }
        else
        {
          printf("   ·   ");
        }

        printf("%s", reset_rename4);
      }

      printf("\n");
      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
      {
        loop_counter[90]++;
        {
          size_t length_rename23 = 40;
          uint8_t value_rename23 = board[x_rename4][y_rename4];
          uint8_t original_rename23[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
          uint8_t blackwhite_rename23[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
          uint8_t bluered_rename23[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
          uint8_t *schemes_rename23[] = {original_rename23, blackwhite_rename23, bluered_rename23};
          uint8_t *background_rename23 = schemes_rename23[scheme] + 0;
          uint8_t *foreground_rename23 = schemes_rename23[scheme] + 1;
          if (value_rename23 > 0)
          {
            loop_counter[91]++;
            while (value_rename23--)
            {
              loop_counter[92]++;
              if ((background_rename23 + 2) < (schemes_rename23[scheme] + (sizeof(original_rename23))))
              {
                loop_counter[93]++;
                background_rename23 += 2;
                foreground_rename23 += 2;
              }

            }

          }

          snprintf(color_rename4, length_rename23, "\e[38;5;%d;48;5;%dm", *foreground_rename23, *background_rename23);
          return23:
          ;

        }
        printf("%s", color_rename4);
        printf("       ");
        printf("%s", reset_rename4);
      }

      printf("\n");
    }

    printf("\n");
    printf("        ←,↑,→,↓ or q        \n");
    printf("\e[A");
    return4:
    ;

  }
  {
    print_loop_counter:
    

    /*
    printf("loop counter = (");
    int i;
    for (i = 0; i < 94; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
    */
    print_array(loop_counter, 94);
  }
  return success;
}

bool main_loop(char c, uint8_t board[SIZE][SIZE], int new_s) {
    bool success;

        switch(c) {
            case 97:    // 'a' key
            case 104:   // 'h' key
            case 68:    // left arrow
                success = moveLeft(board, new_s);  break;
            case 100:   // 'd' key
            case 108:   // 'l' key
            case 67:    // right arrow
                success = moveRight(board, new_s); break;
            case 119:   // 'w' key
            case 107:   // 'k' key
            case 65:    // up arrow
                success = moveUp(board, new_s);    break;
            case 115:   // 's' key
            case 106:   // 'j' key
            case 66:    // down arrow
                success = moveDown(board, new_s);  break;
            default: success = false;
        }
        drawBoard(board, new_s);

    return success;

}

int main(int argc, char *argv[]) {
  uint8_t board[SIZE][SIZE];
  char c;
  bool success;
  srand(0);

  init_time_file();
  //---------------------modified by TJSong----------------------//
  _INIT_();
#if HETERO_EN
  static int current_core = CORE; //0: little, 1: big
  static int is_stable_big = 0; //0: not stable
  static int is_stable_little = 0; //0: not stable
  int pid = getpid();
  llsp_t *solver_big = llsp_new(N_FEATURE + 1);
  llsp_t *solver_little = llsp_new(N_FEATURE + 1);
#elif !HETERO_EN
  llsp_t *solver = llsp_new(N_FEATURE + 1);
#endif
  //---------------------modified by TJSong----------------------//


  if (argc == 2 && strcmp(argv[1],"test")==0) {
    return test();
  }
  if (argc == 2 && strcmp(argv[1],"blackwhite")==0) {
    scheme = 1;
  }
  if (argc == 2 && strcmp(argv[1],"bluered")==0) {
    scheme = 2;
  }

  printf("\e[?25l\e[2J");

  // register signal handler for when ctrl-c is pressed
  signal(SIGINT, signal_callback_handler);
  
  int new_s = SIZE;
  initBoard(board, new_s);
  setBufferedInput(false);
  while (true) {
    //---------------------modified by TJSong----------------------//
    // c=getchar(); //to input automatically
    static int i=0;
    c=65+(i++)%4;
    new_s = 4*((rand())%4)+8;//8~20
    usleep(100000);
    //---------------------modified by TJSong----------------------//

    //---------------------modified by TJSong----------------------//
    fopen_all(); //fopen for frequnecy file
    print_deadline(DEADLINE_TIME); //print deadline 
    //---------------------modified by TJSong----------------------//

    //---------------------modified by TJSong----------------------//
    // Perform slicing and prediction
    struct slice_return predicted_exec_time;
    predicted_exec_time.big = 0;
    predicted_exec_time.little = 0;
    /*
      CASE 0 = to get prediction equation
      CASE 3 = running on default linux governors
      CASE 4 = running on our prediction
      CASE 6 = running on pid
    */
    #if GET_PREDICT /* CASE 0 */
      predicted_exec_time = _SLICE_();
    #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN /* CASE 3 */
      predicted_exec_time = _SLICE_();
      moment_timing_print(0); //moment_start
    #elif !PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN /* CASE 4 */
      moment_timing_print(0); //moment_start
      
      start_timing();
      predicted_exec_time = _SLICE_();
      end_timing();
      slice_time = print_slice_timing();
      
      start_timing();
      #if OVERHEAD_EN //with overhead
        #if HETERO_EN
          current_core = set_freq_hetero(predicted_exec_time.big, 
              predicted_exec_time.little, slice_time, DEADLINE_TIME, 
              AVG_DVFS_TIME, pid, is_stable_big, is_stable_little); //do dvfs
        #else
          #if CORE
            set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
          #else
            set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
          #endif
        #endif
      #endif
      end_timing();
      dvfs_time = print_dvfs_timing();

      moment_timing_print(1); //moment_start
    #elif ORACLE_EN /* CASE 5 */
    #elif PID_EN /* CASE 6 */
      moment_timing_print(0); //moment_start
      
      start_timing();
      predicted_exec_time = pid_controller(exec_time); //pid == slice
      end_timing();
      slice_time = print_slice_timing();
      
      start_timing();
      #if CORE
        set_freq(predicted_exec_time.big, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
      #else
        set_freq(predicted_exec_time.little, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
      #endif
      end_timing();
      dvfs_time = print_dvfs_timing();
      
      moment_timing_print(1); //moment_start
    #elif PROACTIVE_EN /* CASE 7 */
    #endif

    //---------------------modified by TJSong----------------------//
    usleep(100000);
    start_timing();

    success = main_loop(c, board, new_s);
    //success = main_loop_loop_counters(c, board);

    end_timing();
    //---------------------modified by TJSong----------------------//
    _DEFINE_TIME_();

    #if IDLE_EN
      additional_dvfs_times =
        dvfs_table[cur_freq/100000-2][MIN_FREQ/100000-2] +
        dvfs_table[MIN_FREQ/100000-2][cur_freq/100000-2];
    #endif

    #if ONLINE_EN /* CASE 0, 2, 3 and 4 */
      #if GET_PREDICT || GET_OVERHEAD \
            || (!PROACTIVE_EN && !ORACLE_EN && !PID_EN && !PREDICT_EN) \
            || (!PROACTIVE_EN && !ORACLE_EN && !PID_EN && PREDICT_EN) 
        start_timing();
        #if !HETERO_EN
          (void)get_predicted_time(TYPE_SOLVE, solver, NULL, 0, exec_time,
            cur_freq);
        #elif HETERO_EN
          if     (current_core == 1)
            is_stable_big    = get_predicted_time_big   (TYPE_SOLVE, 
                solver_big,    NULL, 0, exec_time, cur_freq);
          else if(current_core == 0)
            is_stable_little = get_predicted_time_little(TYPE_SOLVE, 
                solver_little, NULL, 0, exec_time, cur_freq);
        #endif
        end_timing();
        update_time = exec_timing();
      #endif
    #endif

    _DELAY_();

    _PRINT_INFO_();
    
    fclose_all();//TJSong
    //---------------------modified by TJSong----------------------//

    if (i > 1500)
      break;
    if (success) {
      //drawBoard(board);

      //usleep(150000);
      addRandom(board, new_s);
      drawBoard(board, new_s);
      if (gameEnded(board, new_s)) {
        printf("         GAME OVER          \n");
        break;
      }
    }
    if (c=='q') {
      printf("        QUIT? (y/n)         \n");
      c=getchar();
      if (c=='y') {
        break;
      }
      drawBoard(board, new_s);
    }
    if (c=='r') {
      printf("       RESTART? (y/n)       \n");
      c=getchar();
      if (c=='y') {
        initBoard(board, new_s);
      }
      drawBoard(board, new_s);
    }
  }
  setBufferedInput(true);

  printf("\e[?25h\e[m");

  return EXIT_SUCCESS;
}
