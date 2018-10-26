all:send_server recv_client
	gcc send_server.c -o send_server
	gcc recv_client.c -o recv_client 
clean:
	rm -rf send_server recv_client
