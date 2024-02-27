
#Makefile COR
CFLAGS  = -Wall -pedantic -g -pthread -lpthread

default: all

all:main.o user_interface.o
	gcc $(CFLAGS) -o COR main.o user_interface.o -lgd

main.o: main.c main.h 
	gcc  $(CFLAGS) -c main.c

user_interface.o: user_interface.c user_interface.h 
	gcc  $(CFLAGS) -c user_interface.c


	
clean:
	rm -rf *o all
	rm -rf *o COR
