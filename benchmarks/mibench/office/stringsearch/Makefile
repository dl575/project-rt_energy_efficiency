#FILE1 = bmhasrch.c  bmhisrch.c  bmhsrch.c  pbmsrch_small.c 
FILE2 = solver.h  solver.c  matrix_support.c ldl.c bmhasrch.c  bmhisrch.c  bmhsrch.c  pbmsrch_large.c 

#all: search_small search_large
all: search_large

#search_small: ${FILE1} Makefile
	#gcc -static ${FILE1} -O3 -o search_small
#gcc ${FILE1} -O3 -D_GNU_SOURCE -o search_small

search_large: ${FILE2} Makefile
	#gcc -static ${FILE2} -O3 -o search_large
	gcc ${FILE2} -O3 -Wall -D_GNU_SOURCE -o search_large -lm

clean:
	rm -rf search_small search_large output*
