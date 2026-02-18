CC = gcc
CFLAGS = -Iinclude -I./src -Wall -Wextra -g
LDFLAGS = -lm -lGL -lGLU -lglut

SRC = src/main.c src/tablefan.c src/renderer.c src/math_utils.c
OBJ = $(SRC:.c=.o)
TARGET = tablefan

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean