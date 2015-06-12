CFLAGS  = -std=c11 -Wall -Wextra -O3
LDFLAGS = -pthread

main : main.o lqueue.o wqueue.o sha1.o

main.o : main.c lqueue.h wqueue.h sha1.h
lqueue.o : lqueue.c lqueue.h
wqueue.o : wqueue.c lqueue.h wqueue.h
sha1.o : sha1.c

.PHONY : run clean

run : main
	./$^

clean :
	$(RM) main *.o
