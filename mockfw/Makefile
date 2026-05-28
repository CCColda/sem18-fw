CC = gcc

CFLAGS = -Wall -Wextra -O2 \
	-Iinclude \
	-I$(SDL2_inc)/include/SDL2 \
	-I$(SDL2_ttf)/include

LDFLAGS = \
	-L$(SDL2)/lib \
	-L$(SDL2_ttf)/lib \
	-lSDL2 -lSDL2_ttf

TARGET = app
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)