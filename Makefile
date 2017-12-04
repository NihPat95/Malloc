.Phony : all clean


lib : 
	gcc -g -O0 -Wall -shared -fPIC -pthread -o lib.so ./lib.c ./mybuddy.c

clean : 
	rm -f ./bench ./test ./test2 ./test3 ./test4 ./test5 ./lib.so

test: 
	make test1 && make test2 && make test3 && make test4 && make test5 && make bench

test1:
	gcc -g -w -O0 -o test ./test.c

test2:
	gcc -g -w -O0 -o test2 ./test2.c

test3:
	gcc -g -w -O0 -pthread -o test3 ./test3.c

test4:
	gcc -g -w -O0 -pthread -o test4 ./test4.c

test5:
	gcc -g -w -O0 -pthread -o test5 ./test5.c
bench: 
	gcc -g -pthread -o bench ./bench.c

all: 
	make lib && make test

