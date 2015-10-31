# By default, the code is compiled for a "big endian" machine.
# To compile on a "little endian" machine set the LITTLE_ENDIAN flag.
# To make smaller object code, but run a little slower, don't use UNROLL_LOOPS.
# To use NIST's modified SHA of 7/11/94, define USE_MODIFIED_SHA

CC = gcc
CFLAGS = -O3 -Wall -D_GNU_SOURCE -std=c99

#  2 CFLAGS = -pthread -march=native -O3 -D_XOPEN_SOURCE=600 -D_GNU_SOURCE -std=c99 \


sha:	sha_driver.o sha.o
	$(CC) -o $@ sha_driver.o sha.o -lm
	strip $@

clean:
	rm -rf *.o sha output*
