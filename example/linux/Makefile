# Micro Shell - Linux Makefile

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -g -I../../inc
TARGET  := build/tbsh

SRCS    := ../../src/tbsh.c port_linux.c
OBJS    := $(patsubst %.c,build/%.o,$(notdir $(SRCS)))

# 创建 build 目录
$(shell mkdir -p build)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

build/tbsh.o: ../../src/tbsh.c ../../inc/tbsh.h
	$(CC) $(CFLAGS) -c $< -o $@

build/port_linux.o: port_linux.c ../../inc/tbsh.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

run: $(TARGET)
	./$(TARGET)