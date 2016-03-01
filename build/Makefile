
CFLAGS=-Wall -O3 -g

default:
	gcc -c tokens.c $(CFLAGS)
	gcc -o test test.c tokens.o $(CFLAGS)

clean:
	@rm -f *.o
	@rm -f test
	@echo "Clean!"

