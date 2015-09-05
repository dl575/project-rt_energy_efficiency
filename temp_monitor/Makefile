CC = gcc
CFLAGS = -O3 -Wall

temp_monitor:	temp_monitor.o
	$(CC) -o $@ temp_monitor.o
	strip $@

clean:
	rm -rf *.o temp_monitor
