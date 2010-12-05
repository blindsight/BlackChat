# Blackchat
#  --- Master Makefile
#
  
.PHONY: client server protocol tests clean
  
all: protocol client server tests
  
client:
	cd client; make client
  
server:
	cd server; make server
  
protocol:
	cd protocol; make
  
concurses:
	cd ncurses; ./configure --enable-widec --with-shared --disable-static --enable-ext-colors

curses:
	cd ncurses; make
	mv ncurses/lib/* bin/lib/
	
tests:
	cd protocol; make tests
  
runtests:
	cd bin; ./runtests
clean:
	cd client; make clean
	cd server; make clean
	cd protocol; make clean
	cd ncurses; make clean;
	cd ncurses; rm ncurses/bin/lib/*
	
