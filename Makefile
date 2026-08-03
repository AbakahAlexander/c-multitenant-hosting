CC ?= gcc

CFLAGS = -Wall -Wextra -Werror -Wpedantic -I./db

TEST_TARGET = tests/test_record

.PHONY: all test clean

all: test

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): tests/test_record.c db/record.c db/record.h
	$(CC) $(CFLAGS) tests/test_record.c db/record.c -o $(TEST_TARGET)

clean: 
	rm -f $(TEST_TARGET) *.o
