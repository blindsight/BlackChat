all: protocol.c
	gcc -c -Wall -Wformat-security -Wmissing-prototypes -Wchar-subscripts protocol.c -o protocol.o
	ar rcs libbcprotocol.a protocol.o
	mv libbcprotocol.a ../bin/lib/
	rm protocol.o
clean:
	rm ../bin/lib/libbcprotocol.so.1.0.1
