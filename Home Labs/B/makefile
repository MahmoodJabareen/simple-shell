all: AntiVirus

AntiVirus: AntiVirus.o
	gcc -Wall -g -o AntiVirus AntiVirus.o

AntiVirus.o: AntiVirus.c
	gcc -Wall -g -c -o AntiVirus.o AntiVirus.c

.PHONY: clean all

clean:
	rm -f *.o AntiVirus
