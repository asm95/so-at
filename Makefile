.PHONY: all

OBJ_DIR=obj

all: es hello at

# creates obj directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/topology.o: topology.c
	gcc --std=c99 -c topology.c -o $(OBJ_DIR)/topology.o

# clear object files
clear:
	rm -rf obj/*
	rm -f es at hello

# Message Queue
$(OBJ_DIR)/msg.o: msg/msg.c msg/msg.h common.h
	gcc --std=c99 -c msg/msg.c -o $(OBJ_DIR)/msg.o
$(OBJ_DIR)/msg_tests.o: msg/tests.c msg/tests.h
	gcc --std=c99 -c msg/tests.c -o $(OBJ_DIR)/msg_tests.o

# includes all msg_queue modules
msg_queue: $(OBJ_DIR)/msg.o $(OBJ_DIR)/msg_tests.o

# Main Module

# master
$(OBJ_DIR)/master.o: sch/master.c sch/master.h
	gcc --std=c99 -c sch/master.c -o $(OBJ_DIR)/master.o

# worker
$(OBJ_DIR)/node.o: wrk/node.c wrk/node.h
	gcc --std=c99 -c wrk/node.c -o $(OBJ_DIR)/node.o

$(OBJ_DIR)/jobs.o: sch/jobs.c
	gcc --std=c99 -c sch/jobs.c -o $(OBJ_DIR)/jobs.o

$(OBJ_DIR)/fork.o: fork.c $(OBJ_DIR)/jobs.o $(OBJ_DIR)/master.o $(OBJ_DIR)/node.o
	gcc --std=c99 -c fork.c -o $(OBJ_DIR)/fork.o


es: $(OBJ_DIR) $(OBJ_DIR)/topology.o msg_queue $(OBJ_DIR)/fork.o
	gcc $(OBJ_DIR)/topology.o \
		$(OBJ_DIR)/msg.o $(OBJ_DIR)/msg_tests.o \
		$(OBJ_DIR)/fork.o $(OBJ_DIR)/jobs.o $(OBJ_DIR)/master.o $(OBJ_DIR)/node.o \
		-o es

hello: hello.c
	gcc hello.c -o hello

at: at.c $(OBJ_DIR)/msg.o
	gcc at.c $(OBJ_DIR)/msg.o -o at