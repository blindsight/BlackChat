# Blackchat
#  --- Master Makefile
#
  
.PHONY: client server protocol clean
  
all: protocol client server 

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
	

clean:
	cd client; make clean
	cd server; make clean
	cd protocol; make clean
	cd ncurses; make clean;
	cd ncurses; rm ncurses/bin/lib/*
	
