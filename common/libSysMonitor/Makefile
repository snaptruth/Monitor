###########################################
#Makefile for simple programs
###########################################
VPATH = src
INC= -I../share -I../libOutput

LIB= -lpthread -lrt -ldl

#CC=gcc
#CFLAGS+=-Wall -g $(INC) -fPIC
CFLAGS=-Wall -g $(INC) -fPIC -D_GNU_SOURCE

PRG=libtaskLib.so
OBJ=taskLib.o monitor.o

all: $(PRG)
	
$(PRG): $(OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJ) $(LIB)
	

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG) 


