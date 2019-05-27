.PHONY: all

BIN_DIR=bin
OBJ_DIR=$(BIN_DIR)/obj

all: $(BIN_DIR)/es $(BIN_DIR)/hello $(BIN_DIR)/at

# creates obj directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/topology.o: topology.c
	gcc --std=c99 -c topology.c -o $(OBJ_DIR)/topology.o

# clear object files
clear:
	rm -rf $(BIN_DIR)

# Message Queue
$(OBJ_DIR)/msg.o: msg/msg.c msg/msg.h common.h
	gcc --std=c99 -c msg/msg.c -o $(OBJ_DIR)/msg.o
$(OBJ_DIR)/msg_tests.o: msg/tests.c msg/tests.h
	gcc --std=c99 -c msg/tests.c -o $(OBJ_DIR)/msg_tests.o

# includes all msg_queue modules
msg_queue: $(OBJ_DIR)/msg.o $(OBJ_DIR)/msg_tests.o

# Uitl Module
$(OBJ_DIR)/log.o: util/log.c util/log.h
	gcc --std=c99 -c util/log.c -o $(OBJ_DIR)/log.o

# Main Module

# master
$(OBJ_DIR)/master.o: sch/master.c sch/master.h $(OBJ_DIR)/log.o
	gcc --std=c99 -c sch/master.c -o $(OBJ_DIR)/master.o

# worker
$(OBJ_DIR)/node.o: wrk/node.c wrk/node.h
	gcc --std=c99 -c wrk/node.c -o $(OBJ_DIR)/node.o

$(OBJ_DIR)/jobs.o: sch/jobs.c sch/jobs.h
	gcc --std=c99 -c sch/jobs.c -o $(OBJ_DIR)/jobs.o

$(OBJ_DIR)/fork.o: fork.c sch/jobs.h sch/master.h wrk/node.h
	gcc --std=c99 -c fork.c -o $(OBJ_DIR)/fork.o


$(BIN_DIR)/es: $(OBJ_DIR) $(OBJ_DIR)/topology.o msg_queue $(OBJ_DIR)/fork.o
	gcc $(OBJ_DIR)/topology.o \
		$(OBJ_DIR)/msg.o $(OBJ_DIR)/msg_tests.o \
		$(OBJ_DIR)/fork.o $(OBJ_DIR)/jobs.o $(OBJ_DIR)/master.o $(OBJ_DIR)/node.o \
		$(OBJ_DIR)/log.o \
		-o $(BIN_DIR)/es

$(BIN_DIR)/hello: hello.c
	gcc hello.c -o $(BIN_DIR)/hello

$(BIN_DIR)/at: at.c $(OBJ_DIR)/msg.o
	gcc at.c $(OBJ_DIR)/msg.o -o $(BIN_DIR)/at