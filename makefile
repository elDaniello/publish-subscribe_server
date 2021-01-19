run: compile
	./main.out

compile: clean
	gcc -Wall -g -pthread -o ./main.out ./src/main.c ./src/server.c 

clean:
	echo "XD" > get.out # when compile fails and there's nothig to remove
	rm *.out

test: 
	gcc -o ./tests/test.out ./tests/server_check.c
	./tests/test.out
