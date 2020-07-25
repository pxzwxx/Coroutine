CFLGAS = -I./ -I./share/ -I./ntyco/ -I./serverFile/ -I./clientFile
OBJ = ./ntyco/nty_coroutine.o ./ntyco/nty_schedule.o ./ntyco/nty_socket.o ./ntyco/nty_epoll.o 
 
server = server
client = client

$(server):$(OBJ)
	g++ $? ./serverFile/server.cpp ./share/log.cpp $(CFLGAS) -lpthread -g -o $@
	rm -rf $(OBJ)

$(client):
	g++ ./clientFile/client.cpp ./share/log.cpp $(CFLGAS) -lpthread -g -o $@

$(OBJ):%.o:%.c
	gcc $? $(CFLGAS) -c -o $@

clean:
	rm -rf $(server) $(client) 


