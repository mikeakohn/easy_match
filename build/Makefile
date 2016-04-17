include ../config.mak

#CFLAGS=-Wall -O3 -g
#CFLAGS=-Wall -g
VPATH=../src:../tests

#OBJECTS=tokens.o compiler.o generate_linux_x86_64.o

default: $(OBJECTS) timer.o
	$(CC) -o ../easy_match ../tests/easy_match.c $(OBJECTS) -I../src $(CFLAGS)
	$(CC) -o ../libeasy_match.so $(OBJECTS) -I../src $(CFLAGS) -shared -fPIC

%.o: %.c %.h
	$(CC) -c $< -o $*.o $(CFLAGS) -fPIC

%.o: %.c
	$(CC) -c $< -o $*.o $(CFLAGS) -fPIC

clean:
	@rm -f *.o
	@rm -f test
	@echo "Clean!"

