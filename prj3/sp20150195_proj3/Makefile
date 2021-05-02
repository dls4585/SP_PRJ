CC=gcc
CFLAGS=-g -Wall
TARGET= 20150195.out
OBJECT= 20150195.o func.o assemble.o load.o

$(TARGET) : $(OBJECT)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECT) -lm

20150195.o : 20150195.c
func.o : func.c
assemble.o : assemble.c
load.o : load.c

.PHONY: clean

clean:
	rm *.o *.out
