
#CFLAGS=-Wall -O3 -g
VPATH=../src

OBJECTS=tokens.o

default: $(OBJECTS)

%.o: %.c %.h
	$(CC) -c $< -o $*.o $(CFLAGS)

clean:
	@rm -f *.o
	@rm -f test
	@echo "Clean!"

