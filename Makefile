include config.mak

DEBUG=-DDEBUG -g
CFLAGS=-O3 -Wall $(DEBUG)
CC=gcc
#CC=i686-mingw32-gcc

default:
	@+make -C build

test_tokens:
	$(CC) -o test_tokens tests/test_tokens.c build/tokens.o -Isrc $(CFLAGS)

speed:
	$(CC) -o compare_starts_with tests/compare_starts_with.c build/*.o -Isrc -lpcre $(CFLAGS)
	$(CC) -o compare_ends_with tests/compare_ends_with.c build/*.o -Isrc -lpcre $(CFLAGS)

clean:
	@rm -f build/*.o
	@rm -f easy_match
	@rm -f test_tokens compare_starts_with compare_ends_with



