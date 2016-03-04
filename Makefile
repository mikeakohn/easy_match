DEBUG=-DDEBUG -g
CFLAGS=-O3 -Wall $(DEBUG)
CC=gcc
#CC=i686-mingw32-gcc

default:
	@+make -C build

test_tokens:
	gcc -o test_tokens tests/test_tokens.c build/tokens.o -Isrc $(CFLAGS)

clean:
	@rm -f build/*.o
	@rm -f test_tokens



