.SUFFIXES: .c .o

CC=g++

SRCS1=client.c
SRCS2=server.c	
OBJS1=$(SRCS1:.c=.o)
OBJS2=$(SRCS2:.c=.o)

EXE1=client
EXE2=server

all: $(OBJS1) $(OBJS2)
	$(CC) -o $(EXE1) $(OBJS1) -lpthread
	$(CC) -o $(EXE2) $(OBJS2) -lpthread
	@echo '^_^ ^_^ ^_^ ^_^ ^_^ ^_^ OK ^_^ ^_^ ^_^ ^_^ ^_^ ^_^'

.c.o: 
	$(CC) -Wall -g -o $@ -c $<

clean:
	-rm -f $(OBJS1) $(OBJS2)
	-rm -f core*

