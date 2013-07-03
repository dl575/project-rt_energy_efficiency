/* +++Date last modified: 05-Jul-1997 */

/*
**  BITCNTS.C - Test program for bit counting functions
**
**  public domain by Bob Stout & Auke Reitsma
*/

#include <stdio.h>
#include <stdlib.h>
#include "conio.h"
#include <limits.h>
//#include <time.h>
#include <float.h>
#include "bitops.h"

#include <sys/time.h>

#define FUNCS  7

static int CDECL bit_shifter(long int x);

int main(int argc, char *argv[])
{
  //clock_t start, stop;
  struct timeval start, stop;
  double ct, cmin = DBL_MAX, cmax = 0;
  int i, cminix, cmaxix;
  long j, n, seed;
  int iterations;
  static int (* CDECL pBitCntFunc[FUNCS])(long) = {
    bit_count,
    bitcount,
    ntbl_bitcnt,
    ntbl_bitcount,
    /*            btbl_bitcnt, DOESNT WORK*/
    BW_btbl_bitcount,
    AR_btbl_bitcount,
    bit_shifter
  };
  static char *text[FUNCS] = {
    "Optimized 1 bit/loop counter",
    "Ratko's mystery algorithm",
    "Recursive bit count by nybbles",
    "Non-recursive bit count by nybbles",
    /*            "Recursive bit count by bytes",*/
    "Non-recursive bit count by bytes (BW)",
    "Non-recursive bit count by bytes (AR)",
    "Shift and count bits"
  };
  if (argc<2) {
    fprintf(stderr,"Usage: bitcnts <iterations>\n");
    exit(-1);
	}
  iterations=atoi(argv[1]);
  
  puts("Bit counter algorithm benchmark\n");
  
  gettimeofday(&start, NULL);
  for (i = 0; i < FUNCS; i++) {
    //start = clock();
    gettimeofday(&start, NULL);
    
    for (j = n = 0, seed = rand(); j < iterations; j++, seed += 13)
	 n += pBitCntFunc[i](seed);
    
    /*
    stop = clock();
    ct = (stop - start) / (double)CLOCKS_PER_SEC;
    */

    gettimeofday(&stop, NULL);
    ct = stop.tv_sec - start.tv_sec + (double)(stop.tv_usec - start.tv_usec)/1000000;
    
    if (ct < cmin) {
	 cmin = ct;
	 cminix = i;
    }
    if (ct > cmax) {
	 cmax = ct;
	 cmaxix = i;
    }
    
    printf("iterations = %d\n", iterations);
    printf("%-38s> Time: %7.6f sec.; Bits: %ld\n", text[i], ct, n);
  }
  printf("\nBest  > %s\n", text[cminix]);
  printf("Worst > %s\n", text[cmaxix]);
  return 0;
}

static int CDECL bit_shifter(long int x)
{
  int i, n;
  
  for (i = n = 0; x && (i < (sizeof(long) * CHAR_BIT)); ++i, x >>= 1)
    n += (int)(x & 1L);
  return n;
}
