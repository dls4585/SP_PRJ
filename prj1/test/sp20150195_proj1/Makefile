CC=gcc
CFLAGS=-g -Wall
TARGET= 20150195.out
OBJECT= 20150195.o func.o

$(TARGET) : $(OBJECT)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECT)

20150195.o : 20150195.c
func.o : func.c

.PHONY: clean

clean:
	rm *.o
