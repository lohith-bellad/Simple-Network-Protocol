all: recv send route
	
recv: receiver.o aux.o
	gcc -Wall -o recv receiver.o aux.o
	
send: sender.o aux.o
	gcc -Wall -o send sender.o aux.o
	
route: route.o aux.o
	gcc -Wall -o route route.o aux.o
	
receiver.o: receiver.c custom.h
	gcc -g -Wall -c -o receiver.o receiver.c

sender.o: sender.c custom.h
	gcc -g -Wall -c -o sender.o sender.c
	
route.o: test.c custom.h
	gcc -g -Wall -c -o route.o test.c
	
aux.o: aux.c custom.h
	gcc -g -Wall -c -o aux.o aux.c
	
clean:
	rm *.out *.o

