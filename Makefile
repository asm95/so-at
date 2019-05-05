.PHONY: all

OBJ_DIR=obj

all: es

# creates obj directory
obj_dir:
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/topology.o: topology.c
	gcc --std=c99 -c topology.c -o $(OBJ_DIR)/topology.o

# clear object files
clear:
	rm -rf obj/*
	rm es

# Message Queue
$(OBJ_DIR)/msg.o: msg/msg.c
	gcc --std=c99 -c msg/msg.c -o $(OBJ_DIR)/msg.o
$(OBJ_DIR)/msg_tests.o: msg/tests.c
	gcc --std=c99 -c msg/tests.c -o $(OBJ_DIR)/msg_tests.o

# includes all msg_queue modules
msg_queue: $(OBJ_DIR)/msg.o $(OBJ_DIR)/msg_tests.o


$(OBJ_DIR)/fork.o: fork.c
	gcc --std=c99 -c fork.c -o $(OBJ_DIR)/fork.o


es: obj_dir $(OBJ_DIR)/topology.o msg_queue $(OBJ_DIR)/fork.o
	gcc $(OBJ_DIR)/topology.o \
		$(OBJ_DIR)/msg.o $(OBJ_DIR)/msg_tests.o \
		$(OBJ_DIR)/fork.o -o es