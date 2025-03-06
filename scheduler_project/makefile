scheduler: scheduler.o queue.o
	gcc -g -Wall scheduler.o queue.o -lm -o scheduler

queue.o: queue.h queue.c
	gcc -g -Wall -c queue.c

scheduler.o: queue.h scheduler.c
	gcc -g -Wall -c scheduler.c

clean:
	rm -f queue.o scheduler.o simulator

all: simulator