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

#include "../timing.h"
#include "../my_common.h"

#define SIZE 4
uint32_t score=0;
uint8_t scheme=0;

//---------------------modified by TJSong----------------------//
//set global variables
struct timeval start, end, moment;
int slice_time = 0;
int dvfs_time = 0;

//define benchmarks-depenent varaibles & constants
#if CORE //big
#define OVERHEAD_TIME 3588 //overhead deadline
#define AVG_OVERHEAD_TIME 795 //avg overhead deadline
    #if DEADLINE_DEFAULT
    #define DEADLINE_TIME 42580 + OVERHEAD_TIME //max_exec + max_overhead
    #elif DEADLINE_15MS
    #define DEADLINE_TIME 15000 //15ms
    #endif
#define MAX_DVFS_TIME 2943 //max dvfs time
#define AVG_DVFS_TIME 419 //average dvfs time
#else //LITTLE
#define OVERHEAD_TIME 3651 //overhead deadline
#define AVG_OVERHEAD_TIME 959 //avg overhead deadline
#define DEADLINE_TIME (int)((23931*SWEEP)/100) // max_exec * sweep / 100
#define MAX_DVFS_TIME 3419 //max dvfs time
#define AVG_DVFS_TIME 908 //average dvfs time
#endif
//---------------------modified by TJSong----------------------//

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

void drawBoard(uint8_t board[SIZE][SIZE]) {
	uint8_t x,y;
	char c;
	char color[40], reset[] = "\e[m";
	printf("\e[H");

	printf("2048.c %17d pts\n\n",score);

	for (y=0;y<SIZE;y++) {
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,40);
			printf("%s",color);
			printf("       ");
			printf("%s",reset);
		}
		printf("\n");
		for (x=0;x<SIZE;x++) {
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
		for (x=0;x<SIZE;x++) {
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

bool slideArray(uint8_t array[SIZE]) {
	bool success = false;
	uint8_t x,t,stop=0;

	for (x=0;x<SIZE;x++) {
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

void rotateBoard(uint8_t board[SIZE][SIZE]) {
	uint8_t i,j,n=SIZE;
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

bool moveUp(uint8_t board[SIZE][SIZE]) {
	bool success = false;
	uint8_t x;
	for (x=0;x<SIZE;x++) {
		success |= slideArray(board[x]);
	}
	return success;
}

bool moveLeft(uint8_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveDown(uint8_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveRight(uint8_t board[SIZE][SIZE]) {
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	return success;
}

bool findPairDown(uint8_t board[SIZE][SIZE]) {
	bool success = false;
	uint8_t x,y;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE-1;y++) {
			if (board[x][y]==board[x][y+1]) return true;
		}
	}
	return success;
}

uint8_t countEmpty(uint8_t board[SIZE][SIZE]) {
	uint8_t x,y;
	uint8_t count=0;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			if (board[x][y]==0) {
				count++;
			}
		}
	}
	return count;
}

bool gameEnded(uint8_t board[SIZE][SIZE]) {
	bool ended = true;
	if (countEmpty(board)>0) return false;
	if (findPairDown(board)) return false;
	rotateBoard(board);
	if (findPairDown(board)) ended = false;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return ended;
}

void addRandom(uint8_t board[SIZE][SIZE]) {
	static bool initialized = false;
	uint8_t x,y;
	uint8_t r,len=0;
	uint8_t n,list[SIZE*SIZE][2];

	if (!initialized) {
		//srand(time(NULL));
    srand(0);
		initialized = true;
	}

	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
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

void initBoard(uint8_t board[SIZE][SIZE]) {
	uint8_t x,y;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			board[x][y]=0;
		}
	}
	addRandom(board);
	addRandom(board);
	drawBoard(board);
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
		0,0,0,1,	1,0,0,0,
		0,0,1,1,	2,0,0,0,
		0,1,0,1,	2,0,0,0,
		1,0,0,1,	2,0,0,0,
		1,0,1,0,	2,0,0,0,
		1,1,1,0,	2,1,0,0,
		1,0,1,1,	2,1,0,0,
		1,1,0,1,	2,1,0,0,
		1,1,1,1,	2,2,0,0,
		2,2,1,1,	3,2,0,0,
		1,1,2,2,	2,3,0,0,
		3,0,1,1,	3,2,0,0,
		2,0,1,1,	2,2,0,0
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
		slideArray(array);
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

float main_loop_slice(char c, uint8_t board[4][4])
{
  uint8_t scheme_rename = scheme;
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
        uint8_t n_rename7 = 4;
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
        uint8_t n_rename8 = 4;
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
        uint8_t n_rename9 = 4;
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
        uint8_t n_rename10 = 4;
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
        uint8_t n_rename11 = 4;
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
        uint8_t n_rename12 = 4;
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
        uint8_t n_rename14 = 4;
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
        uint8_t n_rename16 = 4;
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
        uint8_t n_rename17 = 4;
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
        uint8_t n_rename19 = 4;
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
        uint8_t n_rename20 = 4;
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
    for (y_rename4 = 0; y_rename4 < 4; y_rename4++)
    {
      loop_counter[81]++;
      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
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

      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
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

      for (x_rename4 = 0; x_rename4 < 4; x_rename4++)
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
      write_array(loop_counter, 95);
#endif
  }
  {
    predict_exec_time:
    ;

    float exec_time;
#if CORE //big
    exec_time = 1315.600000*loop_counter[2] + 21.085700*loop_counter[8] + 23.628600*loop_counter[9] + -29.657100*loop_counter[10] + -82.257100*loop_counter[12] + 10.742900*loop_counter[14] + 27.542900*loop_counter[15] + 1311.930000*loop_counter[24] + 35.809100*loop_counter[34] + 11.469700*loop_counter[35] + -20.100000*loop_counter[36] + -40.954500*loop_counter[38] + 4.806060*loop_counter[40] + 14.848500*loop_counter[41] + 1153.560000*loop_counter[46] + 75.091600*loop_counter[50] + 43.063600*loop_counter[51] + -52.476600*loop_counter[53] + 75.106500*loop_counter[54] + -47.530800*loop_counter[56] + -134.849000*loop_counter[57] + 1492.870000*loop_counter[60] + -3.000000*loop_counter[68] + -6.266670*loop_counter[69] + -5.422220*loop_counter[70] + -39.155600*loop_counter[72] + 18.822200*loop_counter[74] + 31.822200*loop_counter[75] + 0.800000*loop_counter[84] + 0.000000;
#else //LITTLE
    exec_time = 15975.600000*loop_counter[2] + -4812.610000*loop_counter[8] + -403.494000*loop_counter[9] + 1905.650000*loop_counter[11] + 4120.520000*loop_counter[12] + -3311.910000*loop_counter[14] + -2053.130000*loop_counter[15] + 13267.000000*loop_counter[24] + -308.201000*loop_counter[34] + -287.788000*loop_counter[35] + 529.847000*loop_counter[36] + -239.121000*loop_counter[38] + 293.302000*loop_counter[40] + -45.485300*loop_counter[41] + 13347.200000*loop_counter[46] + 149.632000*loop_counter[50] + 45.845900*loop_counter[51] + -222.743000*loop_counter[53] + -387.524000*loop_counter[54] + 152.997000*loop_counter[56] + 7162.930000*loop_counter[60] + -4120.070000*loop_counter[68] + -46.884600*loop_counter[69] + 2248.650000*loop_counter[71] + -1662.160000*loop_counter[73] + 5149.290000*loop_counter[75] + 24.212700*loop_counter[84] + 0.000000;
#endif
    return exec_time;
  }
}

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
      write_array(loop_counter, 95);
#endif

  }
  {
    predict_exec_time:
    ;

    float exec_time;
    exec_time = 1315.600000*loop_counter[2] + 21.085700*loop_counter[8] + 23.628600*loop_counter[9] + -29.657100*loop_counter[10] + -82.257100*loop_counter[12] + 10.742900*loop_counter[14] + 27.542900*loop_counter[15] + 1311.930000*loop_counter[24] + 35.809100*loop_counter[34] + 11.469700*loop_counter[35] + -20.100000*loop_counter[36] + -40.954500*loop_counter[38] + 4.806060*loop_counter[40] + 14.848500*loop_counter[41] + 1153.560000*loop_counter[46] + 75.091600*loop_counter[50] + 43.063600*loop_counter[51] + -52.476600*loop_counter[53] + 75.106500*loop_counter[54] + -47.530800*loop_counter[56] + -134.849000*loop_counter[57] + 1492.870000*loop_counter[60] + -3.000000*loop_counter[68] + -6.266670*loop_counter[69] + -5.422220*loop_counter[70] + -39.155600*loop_counter[72] + 18.822200*loop_counter[74] + 31.822200*loop_counter[75] + 0.800000*loop_counter[84] + 0.000000;
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
      write_array(loop_counter, 95);
#endif

  }
  {
    predict_exec_time:
    ;

    float exec_time;
    exec_time = 15975.600000*loop_counter[2] + -4812.610000*loop_counter[8] + -403.494000*loop_counter[9] + 1905.650000*loop_counter[11] + 4120.520000*loop_counter[12] + -3311.910000*loop_counter[14] + -2053.130000*loop_counter[15] + 13267.000000*loop_counter[24] + -308.201000*loop_counter[34] + -287.788000*loop_counter[35] + 529.847000*loop_counter[36] + -239.121000*loop_counter[38] + 293.302000*loop_counter[40] + -45.485300*loop_counter[41] + 13347.200000*loop_counter[46] + 149.632000*loop_counter[50] + 45.845900*loop_counter[51] + -222.743000*loop_counter[53] + -387.524000*loop_counter[54] + 152.997000*loop_counter[56] + 7162.930000*loop_counter[60] + -4120.070000*loop_counter[68] + -46.884600*loop_counter[69] + 2248.650000*loop_counter[71] + -1662.160000*loop_counter[73] + 5149.290000*loop_counter[75] + 24.212700*loop_counter[84] + 0.000000;
    return exec_time;
  }
}
#endif

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
    write_array(loop_counter, 94);
  }
  return success;
}

bool main_loop(char c, uint8_t board[SIZE][SIZE]) {
    bool success;

		switch(c) {
			case 97:	// 'a' key
			case 104:	// 'h' key
			case 68:	// left arrow
				success = moveLeft(board);  break;
			case 100:	// 'd' key
			case 108:	// 'l' key
			case 67:	// right arrow
				success = moveRight(board); break;
			case 119:	// 'w' key
			case 107:	// 'k' key
			case 65:	// up arrow
				success = moveUp(board);    break;
			case 115:	// 's' key
			case 106:	// 'j' key
			case 66:	// down arrow
				success = moveDown(board);  break;
			default: success = false;
		}
		drawBoard(board);

    return success;

}

int main(int argc, char *argv[]) {
	uint8_t board[SIZE][SIZE];
	char c;
	bool success;

  //init_time_file();

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

	initBoard(board);
	setBufferedInput(false);
	while (true) {
//---------------------modified by TJSong----------------------//
        // c=getchar(); //to input automatically
        static int i=0;
		c=65+(i++)%4;
//---------------------modified by TJSong----------------------//

//---------------------modified by TJSong----------------------//
    fopen_all(); //fopen for frequnecy file
    fprint_deadline(DEADLINE_TIME); //print deadline 
//---------------------modified by TJSong----------------------//

//---------------------modified by TJSong----------------------//
    // Perform slicing and prediction
    float predicted_exec_time = 0.0;
    /*
        CASE 0 = to get prediction equation
        CASE 1 = to get execution deadline
        CASE 2 = to get overhead deadline
        CASE 3 = running on default linux governors
        CASE 4 = running on our prediction 
    */
    #if GET_PREDICT /* CASE 0 */
        predicted_exec_time = main_loop_slice_reduced(c, board); //slice
    #endif
    #if GET_DEADLINE /* CASE 1 */
        //nothing
    #endif
    #if GET_OVERHEAD /* CASE 2 */
        start_timing();
        predicted_exec_time = main_loop_slice_reduced(c, board); //slice
        end_timing();
        slice_time = fprint_slice_timing();

        start_timing();
        set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
        end_timing();
        dvfs_time = fprint_dvfs_timing();
    #endif
    #if !GET_PREDICT && !GET_DEADLINE && !GET_OVERHEAD && !PREDICT_EN /* CASE 3 */
        //slice_time=0; dvfs_time=0;
        moment_timing_fprint(0); //moment_start
    #endif
    #if !GET_PREDICT && !GET_DEADLINE && !GET_OVERHEAD && PREDICT_EN /* CASE 4 */
        moment_timing_fprint(0); //moment_start
        
        start_timing();
        predicted_exec_time = main_loop_slice_reduced(c, board); //slice
        end_timing();
        slice_time = fprint_slice_timing();
        
        moment_timing_fprint(1); //moment_start

        start_timing();
        set_freq(predicted_exec_time, slice_time, DEADLINE_TIME, AVG_DVFS_TIME); //do dvfs
        end_timing();
        dvfs_time = fprint_dvfs_timing();
    #endif
    
    // Write out predicted time & print out frequency used
    #if DEBUG_EN
        fprint_predicted_time(predicted_exec_time);
        fprint_freq(); //[DEBUG] check frequency 
    #endif
//---------------------modified by TJSong----------------------//

    start_timing();

    success = main_loop(c, board);
    //success = main_loop_loop_counters(c, board);

    end_timing();
//---------------------modified by TJSong----------------------//
    int exec_time = exec_timing();
    int delay_time = 0;

    #if GET_PREDICT /* CASE 0 */
        fprint_exec_time(exec_time);
    #endif
    #if GET_DEADLINE /* CASE 1 */
        fprint_exec_time(exec_time);
    #endif
    #if GET_OVERHEAD /* CASE 2 */
        //nothing
    #endif
    #if !GET_PREDICT && !GET_DEADLINE && !GET_OVERHEAD /* CASE 3 and 4 */
        if(DELAY_EN && ((delay_time = DEADLINE_TIME - exec_time - slice_time - dvfs_time) > 0)){
            start_timing();
            usleep(delay_time);
            end_timing();
            delay_time = exec_timing();
        }else
            delay_time = 0;
        fprint_total_time(exec_time + slice_time + dvfs_time + delay_time);
        moment_timing_fprint(2); //moment_end
    #endif
    fclose_all();//TJSong
//---------------------modified by TJSong----------------------//

		if (success) {
			//drawBoard(board);

			//usleep(150000);
			addRandom(board);
			drawBoard(board);
			if (gameEnded(board)) {
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
			drawBoard(board);
		}
		if (c=='r') {
			printf("       RESTART? (y/n)       \n");
			c=getchar();
			if (c=='y') {
				initBoard(board);
			}
			drawBoard(board);
		}
	}
	setBufferedInput(true);

	printf("\e[?25h\e[m");

	return EXIT_SUCCESS;
}
