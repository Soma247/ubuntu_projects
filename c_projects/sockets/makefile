default: getaddr
clean: 
	rm getaddr client server *.o
getaddr: getaddr.c getaddr.h
	gcc getaddr.c -o getaddr
server: err_fun.o server.o
	gcc server.o err_funcs.o -o server
client: err_fun.o client.o
	gcc client.o err_funcs.o -o client
err_fun.o: ../err_funcs.c ../err_funcs.h
	gcc -c ../err_funcs.c 
server.o: server.c server.h ../daemonize/daemonize.h
	gcc -c server.c 
.PHONY: default
