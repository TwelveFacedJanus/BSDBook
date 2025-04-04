CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -lncurses -ljansson
SRC_DIR = src
LIB_DIR = lib
BIN_DIR = bin

all: $(BIN_DIR)/bsdnotes

$(BIN_DIR)/bsdnotes: $(SRC_DIR)/main.c $(LIB_DIR)/libbsdcore.so
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/main.c $(SRC_DIR)/bsdcore.c -o $@ -L$(LIB_DIR) -lbsdcore $(LDFLAGS)

# Dynamic library
$(LIB_DIR)/libbsdcore.so: $(SRC_DIR)/bsdcore.c
	mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) -shared $< -o $@ $(LDFLAGS)

# Static library
$(LIB_DIR)/libbsdcore.a: $(SRC_DIR)/bsdcore.c
	mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) -c $< -o $(LIB_DIR)/bsdcore.o
	ar rcs $@ $(LIB_DIR)/bsdcore.o

libs: $(LIB_DIR)/libbsdcore.so $(LIB_DIR)/libbsdcore.a

install:
	mkdir -p $(HOME)/books
	cp $(BIN_DIR)/bsdnotes /usr/local/bin/
	cp $(LIB_DIR)/libbsdcore.so /usr/local/lib/
	cp $(LIB_DIR)/libbsdcore.a /usr/local/lib/
	ldconfig

clean:
	rm -f $(BIN_DIR)/bsdnotes $(LIB_DIR)/*.o $(LIB_DIR)/*.a $(LIB_DIR)/*.so

.PHONY: all libs install clean
