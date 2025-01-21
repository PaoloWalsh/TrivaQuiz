CC = gcc
CFLAGS = -Wall -c -g
LDFLAGS = 

SRCS = src/server.c src/theme.c src/vector.c src/client-info.c src/msg-exchange.c \
       src/client.c src/client-messages.c src/state.c
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))

all: server client

server: build/server.o build/theme.o build/vector.o build/client-info.o build/msg-exchange.o build/state.o
	$(CC) $^ -o $@

client: build/client.o build/client-messages.o build/msg-exchange.o
	$(CC) $^ -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build server client
