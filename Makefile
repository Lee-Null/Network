CC = gcc

main : server.o http.o
	gcc -o main server.o http.o

server.o : server.c
	gcc -o server.o -c server.c

http.o : http.c
	gcc -o http.o -c http.c

clean : 
	rm *.o
	rm main