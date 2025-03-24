all:
	gcc13 src/main.c -o bsdnotes -lncurses

install:
	mkdir \($HOME)/books
	cp bsdnotes /usr/local/bin/
