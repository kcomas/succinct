CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra
SRC = ./src
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
NAME = sc

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(SRC)/*.o
	rm -f $(NAME)
