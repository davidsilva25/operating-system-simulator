CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -I src
SRC_DIR = src
BUILD_DIR = build
SOURCES = main.c inputs.c memoria.c escalonador.c executor.c output.c
OBJECTS = $(addprefix $(BUILD_DIR)/,$(SOURCES:.c=.o))
TARGET = simulador

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/%.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

clean:
	rm -rf $(BUILD_DIR)/*
	rm -f $(TARGET) src/*.o src/simulador

.PHONY: all clean