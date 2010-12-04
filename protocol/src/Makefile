all: protocol.c
	gcc -c -fPIC -Wall -Wformat-security -Wmissing-prototypes -Wchar-subscripts protocol.c -o protocol.o
	gcc -shared -W1,-sobcpro,libbcprotocol.so.1 -o libbcprotocol.so.1.0.1 protocol.o
	mv libbcprotocol.so.1.0.1 ../../bin/lib/
	rm protocol.o
clean:
	rm ../../bin/lib/libbcprotocol.so.1.0.1
