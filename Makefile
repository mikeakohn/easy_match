DEBUG=-DDEBUG -g
CFLAGS=-O3 -Wall $(DEBUG)
CC=gcc
#CC=i686-mingw32-gcc

default:
	@+make -C build

test_tokens:
	$(CC) -o test_tokens tests/test_tokens.c build/tokens.o -Isrc $(CFLAGS)

speed:
	$(CC) -o compare_startswith tests/compare_startswith.c build/*.o -Isrc -lpcre $(CFLAGS)
	$(CC) -o compare_endswith tests/compare_endswith.c build/*.o -Isrc -lpcre $(CFLAGS)

clean:
	@rm -f build/*.o
	@rm -f test_tokens compare_startswith compare_endswith



