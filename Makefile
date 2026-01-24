CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude
LDFLAGS = -pthread

SRC = src/main.c src/db.c
OUT = hosty-db

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) $(LDFLAGS)

clean:
	rm -f $(OUT) *.o data.wal