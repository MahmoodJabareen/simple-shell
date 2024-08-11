all: loader


loader: start.o startup.o
	gcc -m32 -c loader.c -o loader.o
	ld -o loader loader.o startup.o start.o -L/usr/lib32 -lc -T linking_script -dynamic-linker /lib32/ld-linux.so.2

start.o:
	nasm -g -f elf32 -o start.o start.s

startup.o:
	nasm -g -f elf32 -o startup.o startup.s

clean:
	rm -f *.o loader
