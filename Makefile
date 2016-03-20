include config.mak

DEBUG=-DDEBUG -g
#CFLAGS=-O3 -Wall $(DEBUG)
CC=gcc
#CC=i686-mingw32-gcc

default:
	@+make -C build

test_tokens:
	$(CC) -o test_tokens tests/test_tokens.c build/tokens.o -Isrc $(CFLAGS)

compare:
	$(CC) -o compare_speed tests/compare_speed.c build/*.o -Isrc -lpcre -lrt $(CFLAGS)

clean:
	@rm -f build/*.o
	@rm -f easy_match
	@rm -f test_tokens compare_speed



