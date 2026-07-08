CC = gcc
CFLAGS = -Wall -O2 -Isrc -I../shared
LDFLAGS = -lX11

SRC = src/*.c
OUT = irisWM

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)

.PHONY: all clean