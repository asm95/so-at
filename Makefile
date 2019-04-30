.PHONY: all

all: es

topology.o: topology.c
	gcc --std=c99 -c topology.c -o topology.o

fork.o: fork.c
	gcc --std=c99 -c fork.c -o fork.o

es: fork.o topology.o
	gcc fork.o topology.o -o es