all:
	gcc13 src/main.c -o bsdnotes -lncurses

install:
	mkdir ~/bsdbook_books
	cp bsdnotes /usr/local/bin/
