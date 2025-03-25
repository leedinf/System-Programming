
testlib : main.o bitmap.o debug.o hash.o list.o hex_dump.o
	gcc -o testlib main.o bitmap.o debug.o hash.o list.o hex_dump.o

bitmap.o : bitmap.c
	gcc -c -o bitmap.o bitmap.c

main.o : main.c
	gcc -c -o main.o main.c

debug.o : debug.c
	gcc -c -o debug.o debug.c
	
hash.o : hash.c
	gcc -c -o hash.o hash.c

list.o : list.c
	gcc -c -o list.o list.c
	
hex_dump.o : hex_dump.c
	gcc -c -o hex_dump.o hex_dump.c

clean:
	rm *.o testlib
	