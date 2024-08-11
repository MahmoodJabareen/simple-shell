all: start

start: start.o util.o
	gcc -m32 util.o start.o -o start

start.o: start.s
	nasm -f elf32 start.s -o start.o
util.o:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

clean:
	rm -f *.o start

# all: task0

# task0: start.o main.o util.o
# 	ld -m elf_i386 -e main start.o main.o util.o -o task0

# start.o: start.s
# 	nasm -f elf32 start.s -std=c99 -o start.o
# util.o:
# 	gcc -m32 -Wall -ansi -c -std=c99 -nostdlib -fno-stack-protector util.c -o util.o

# main.o:
# 	gcc -m32 -Wall -ansi -c -std=c99 -nostdlib -fno-stack-protector main.c -o main.o
# clean:
# 	rm -f *.o task0