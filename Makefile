.PHONY: test

main:
	gcc main.c -o main

release:
	gcc main.c -o main -O3

test:
	gcc tests.c -o tests
	./tests

