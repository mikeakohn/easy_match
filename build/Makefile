
#CFLAGS=-Wall -O3 -g
CFLAGS=-Wall -g
VPATH=../src

OBJECTS=tokens.o compiler.o generate_linux_x86_64.o

default: $(OBJECTS)
	$(CC) -o ../easy_match ../tests/easy_match.c $(OBJECTS) -I../src $(CFLAGS)

%.o: %.c %.h
	$(CC) -c $< -o $*.o $(CFLAGS)

clean:
	@rm -f *.o
	@rm -f test
	@echo "Clean!"

