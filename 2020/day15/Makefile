default_target: main

CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -pedantic -O3
INCLUDE_FLAGS = -I/home/dobby/clib
SRC_DIR = src 

main: main.c
	$(CC) $(INCLUDE_FLAGS) $(CFLAGS) $^ -o aoc
