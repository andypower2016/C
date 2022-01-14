CC      := gcc
INC     := -I"./include"
CFLAGS  := -O -Wall -pthread $(INC)

SRC 	:= ./
SRCS    := $(shell find $(SRC) -name '*.c')

OBJ 	:= ./
OBJS    := $(SRCS:%.c=%.o)

BIN 	:= main
LDLIBS	:= -lm -lrt
MKDIR   := mkdir
RMDIR   := rmdir

all:$(OBJS) 
	$(CC) -o $(BIN) $^ $(CFLAGS) $(LDLIBS)
	@echo make target [$@] is complete

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) 
	@echo make target [$@] is complete

clean:
	-rm -f $(BIN) $(OBJS)



