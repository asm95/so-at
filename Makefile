.PHONY: all

all: es

topology.o: topology.c
	gcc --std=c99 -c topology.c -o topology.o

msg.o: msg.c
	gcc --std=c99 -c msg.c -o msg.o

fork.o: fork.c
	gcc --std=c99 -c fork.c -o fork.o

es: topology.o msg.o fork.o
	gcc topology.o  msg.o fork.o -o es