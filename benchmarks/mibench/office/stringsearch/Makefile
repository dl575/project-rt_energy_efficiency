FILE2 = bmhasrch.c  bmhisrch.c  bmhsrch.c  pbmsrch_large.c 

all: search_large

search_large: ${FILE2} Makefile
	gcc ${FILE2} -O3 -D_GNU_SOURCE -std=c99 -o search_large -lm

clean:
	rm -rf search_small search_large output*
