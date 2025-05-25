all: myshell mypipeline

myshell: myshell.o LineParser.o
	gcc -m32 -Wall -g -o myshell myshell.o LineParser.o

myshell.o: myshell.c LineParser.h
	gcc -m32 -Wall -g -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -m32 -Wall -g -c -o LineParser.o LineParser.c

mypipeline: mypipeline.o
	gcc -m32 -Wall -g -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -m32 -Wall -g -c -o mypipeline.o mypipeline.c

.PHONY: clean all

clean:
	rm -f *.o myshell mypipeline
