run: compile
	./main.out

compile: clean
	gcc -pthread main.c -o main.out -g

clean: 
	rm *.out