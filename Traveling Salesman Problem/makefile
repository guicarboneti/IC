# Copyright 2021, Gurobi Optimization, LLC

PLATFORM = linux64
TWOUP    = /opt/gurobi951/linux64
KEEPTU   = ../..
INC      = $(TWOUP)/include/
CC       = gcc
CARGS    = -m64 -g
CLIB     = -L$(TWOUP)/lib -lgurobi95

all: tsp

tsp: TSP.c
	$(CC) $(CARGS) -o $@ $< -I$(INC) $(CLIB) -lm -Wall

clean:
	rm -rf *.o *_c
