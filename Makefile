CC=gcc
FLAGS=-Wall -Wextra -pedantic

build: src/* out/
	$(CC) $(FLAGS) -o out/simple-shell src/*

out/:
	mkdir out/

run: build
	./out/simple-shell

clean:
	rm -r out/
