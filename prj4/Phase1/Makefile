CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = shellex
OBJECT = myshell.o csapp.o

$(PROGS) : $(OBJECT)
	$(CC) $(CFLAGS) $(LDLIBS) -o $(PROGS) $(OBJECT)

myshell.o: myshell.c
csapp.o: csapp.c

clean:
	rm -rf *~ $(PROGS) *.o


