CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lX11

SRC = src/*.c
OUT = irisWM

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)

run: all
	./$(OUT)

.PHONY: all clean run