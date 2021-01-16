run: compile
	./main.out

compile: clean
	gcc -pthread main.c -o main.out -g

clean:
	echo "XD" > get.out # when compile fails and there's nothig to remove
	rm *.out