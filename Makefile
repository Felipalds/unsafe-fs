.PHONY: main

main:
	gcc main.c -o main -Wall

release:
	gcc main.c -o main -O3 -Wall

test:
	mkdir -p ./images
	gcc tests.c -o tests
	./tests

