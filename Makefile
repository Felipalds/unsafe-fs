.PHONY: main

main:
	gcc main.c -o main

release:
	gcc main.c -o main -O3

test:
	mkdir -p ./images
	gcc tests.c -o tests
	./tests

