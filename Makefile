#Makefile
CC=g++
INCLUDE=
LIB=-lpthread
CFLAGS=-g -Wall -Werror -D_REENTRANT -D_GNU_SOURCE ${LIB} ${INCLUDE}
target+=$(patsubst %.c, %.o, ${src})
springcleaning=$(patsubst %.c, %, $(wildcard ./*.c))
springcleaning+=$(patsubst %.c, %.o, $(wildcard ./*.c))
springcleaning+=$(patsubst %.c, %.o, ${src})
springcleaning+=$(OutPut)

.PHONY: all clean for_main

all: for_main

for_main:
	$(CC) -g main.cpp acm_encryption.c json.c aes.c encode_decode.c -o main -lpthread 
	
clean:
	-@rm  *.o main
