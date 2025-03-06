http_client: 
	cd clientFiles; gcc -Wall http_client.c -o http_client

http_server: 
	cd serverFiles; gcc -Wall http_server.c -o http_server

all: http_server http_client
	make http_server
	make http_client

clean: 
	rm -f serverFiles/http_server 
	rm -f clientFiles/http_client
