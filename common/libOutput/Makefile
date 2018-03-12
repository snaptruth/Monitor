###########################################
#Makefile for simple programs
###########################################
VPATH = src
INC= -I../share

LIB=  -lpthread -lrt 

#CC=gcc
#CFLAGS+=-Wall -g $(INC) -fPIC
CFLAGS=-Wall -g $(INC) -fPIC

PRG=liboutputLib.so
OBJ=outputLib.o

all: $(PRG)
	
$(PRG): $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJ) $(LIB)
	
.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG) 



