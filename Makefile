all:
	gcc src/main.c -o bsdnotes -lncurses

install:
	mkdir -p $(HOME)/books
	cp -r bsdnotes /usr/local/bin/
