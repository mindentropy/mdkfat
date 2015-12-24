INC_DIR = include

OBJ_DIR := obj
SRC_DIR := src
BIN_DIR := bin
LDFLAGS = -L.

FAT_BIN = $(BIN_DIR)/fat_test

CC = gcc
LD = ld

all: cscope_create fat_test

SRC_FILES := $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.c))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

CFLAGS = -g -I$(INC_DIR)

fat_test: $(OBJ_DIR) $(BIN_DIR) $(OBJ_FILES) $(FAT_BIN)

$(FAT_BIN): $(OBJ_FILES)
	$(CC) -Wall -o $(FAT_BIN) $(OBJ_FILES) $(LDFLAGS)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

cscope_create:
	cscope -R -q -b

cleanall: clean cleandir cscope_clean

clean:
	rm -fv $(BIN_DIR)/*
	rm -fv $(OBJ_DIR)/*

cleandir:
	rm -df $(BIN_DIR)
	rm -df $(OBJ_DIR)

cscope_clean:
	rm -fv *.out

