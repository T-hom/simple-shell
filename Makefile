CC=gcc
FLAGS=-Wall -Wextra -pedantic

build: src/main.c out/
	$(CC) $(FLAGS) -o out/simple-shell src/main.c

out/:
	mkdir out/

run: build
	./out/simple-shell

clean:
	rm -r out/
