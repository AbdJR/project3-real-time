project3: project3b.o 
	gcc -o project3 project3b.o -lpthread

project3b.o: project3b.c local.h
	gcc -g -Wunused -lpthread -c project3b.c 

.PHONY: clean

clean:
	rm -rf *.o $(OUT)
