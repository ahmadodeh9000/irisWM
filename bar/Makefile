CC = gcc
CFLAGS = -Wall -O2 -Isrc -I../shared $(shell pkg-config --cflags freetype2)
LDFLAGS = -lX11 -lXft -lfontconfig

SRC = src/*.c
OUT = irisBar

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)

.PHONY: all clean