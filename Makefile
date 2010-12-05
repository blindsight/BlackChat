# Blackchat
#  --- Master Makefile
#

.PHONY: client server protocol tests clean

all: client server protocol tests

client:
	cd client; make client

server:
	cd server; make server

protocol:
	cd protocol; make protocol 

tests:
	cd test; make tests

runtests:
	cd bin; ./runtests
clean:
	cd client; make clean
	cd server; make clean
	cd protocol; make clean
