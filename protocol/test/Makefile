.PHONY: clean

all: tester.c
	gcc -Wall -I../src/  tester.c -o test ../../bin/lib/libbcprotocol.so.1.0.1
	mv test ../../bin/
clean:
	rm ../../bin/test
