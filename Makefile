CC=gcc
FLAGS=-Wall -Wextra -pedantic

out/simple-shell: src/*
	mkdir -p out/
	$(CC) $(FLAGS) -o out/simple-shell src/*

run: out/simple-shell
	./out/simple-shell

clean:
	rm -r out/
