CC = gcc
CFLAGS = -O3 -Wall

power_monitor:	power_monitor.o
	$(CC) -o $@ power_monitor.o
	strip $@

clean:
	rm -rf *.o power_monitor
