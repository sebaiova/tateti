CC=gcc
CFLAGS=-Wall
SERVER_SRC=server.c
CLIENT_SRC=client.c
IP=127.0.0.1
PORT=2000

BIN=./bin
SOURCE=./

all: $(BIN)/client $(BIN)/server

$(BIN)/%: $(SOURCE)%.c
	@mkdir -p $(BIN)
	$(CC) $(INC) $< $(CFLAGS) -o $@

run-server:
	$(BIN)/server $(PORT)

run-client:
	$(BIN)/client $(IP) $(PORT)

clean:
	rm -rf $(BIN)/*
