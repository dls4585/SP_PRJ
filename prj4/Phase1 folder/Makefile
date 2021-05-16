CC = gcc
CFLAGS = -Og
LDLIBS = -lpthread

PROGS = shellex
OBJECT = myshell.o  myshell_func.o csapp.o

$(PROGS) : $(OBJECT)
	$(CC) $(CFLAGS) -o $(PROGS) $(OBJECT) $(LDLIBS)

myshell.o: myshell.c
myshell_func.o: myshell_func.c
csapp.o: csapp.c

clean:
	rm -rf *~ $(PROGS) *.o


