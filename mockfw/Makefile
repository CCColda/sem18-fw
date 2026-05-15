CC = gcc
CFLAGS = -Wall -Wextra -O2 \
	-I$(SDL2_inc)/include \
	-I$(SDL2_inc)/include/SDL2 \
	-I$(SDL2_ttf)/include \
	-I$(SDL2_ttf)/include/SDL2

LDFLAGS = \
	-L$(SDL2)/lib \
	-L$(SDL2_ttf)/lib \
	-lSDL2 -lSDL2_ttf

TARGET = app
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)