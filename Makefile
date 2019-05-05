.PHONY: all

OBJ_DIR=obj

all: es

obj_dir:
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/topology.o: topology.c
	gcc --std=c99 -c topology.c -o $(OBJ_DIR)/topology.o

$(OBJ_DIR)/msg.o: msg/msg.c
	gcc --std=c99 -c msg/msg.c -o $(OBJ_DIR)/msg.o

$(OBJ_DIR)/fork.o: fork.c
	gcc --std=c99 -c fork.c -o $(OBJ_DIR)/fork.o

es: obj_dir $(OBJ_DIR)/topology.o $(OBJ_DIR)/msg.o $(OBJ_DIR)/fork.o
	gcc $(OBJ_DIR)/topology.o $(OBJ_DIR)/msg.o $(OBJ_DIR)/fork.o -o es