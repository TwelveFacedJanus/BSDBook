all:
	gcc13 src/main.c -o bsdnotes -lncurses

install:
	cp bsdnotes /usr/local/bin/
