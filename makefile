all:
	gcc server.c  msglib/msglib.c -o server.exe
	gcc client.c  msglib/msglib.c -o client.exe
clean:
	rm client.exe
	rm server.exe

