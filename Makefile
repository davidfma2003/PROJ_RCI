#Makefile COR
CFLAGS  = -Wall -pedantic -g

default: all

all: main.o user_interface.o
	gcc  $(CFLAGS) -o COR main.o user_interface.o

main.o: main.c main.h 
	gcc  $(CFLAGS) -c main.c

user_interface.o: user_interface.c user_interface.h 
	gcc  $(CFLAGS) -c user_interface.c

debug: CFLAGS += -DDEBUG
debug: all

clean:
	rm -rf *o all
	rm -rf *o COR