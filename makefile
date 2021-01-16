run: compile
	./main.out

compile: clean
	gcc -pthread -o main.out main.c server.c

clean:
	echo "XD" > get.out # when compile fails and there's nothig to remove
	rm *.out