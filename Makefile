CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
TARGET = maintenance_optimizer
SOURCES = main.c dp_optimizer.c utils.c results.c comparison.c
OBJECTS = $(SOURCES:.c=.o)

all: clean $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm

%.o: %.c main.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) optimization_results.txt

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@echo "=== TEST 1: 5 Makine, 30 Gun ==="
	@echo "5\n30\n" | ./$(TARGET) > test_output.txt
	@echo "\n=== TEST 2: 10 Makine, 30 Gun ==="
	@echo "10\n30\n" | ./$(TARGET) >> test_output.txt

.PHONY: all clean run test
