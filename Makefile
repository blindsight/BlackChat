# Blackchat
#  --- Master Makefile
#

all: client server

client:
	make -w -B -C client

server:
	cd server; make

clean:
	cd client; client clean
	cd server; server clean


