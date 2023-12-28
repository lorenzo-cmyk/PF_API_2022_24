CC = gcc
CFLAGS = -DEVAL -Wall -Werror -std=gnu11 -O2 -pipe -static -s -g3
LDFLAGS = -lm

TARGET = main.o
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
