all: first

first: first.o
	gcc -Wall -Werror -fsanitize=address first.o -o first -lm

clean:
	rm -rf first.o first
