#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h> /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#define MAXPENDING 5 /* Maximum outstanding connection requests */
#define BUFFER_SIZE 1024
#define RESPONSE_CHUNCK_SIZE 2048
#define MAX_FILEPATH_LENGTH 128
void DieWithError(char *errorMessage); /* Error handling function */
void HandleTCPClient(int clntSocket); /* TCP client handling function */

int main(int argc, char *argv[])
{ 
int servSock; /*Socket descriptor for server */
int clntSock; /* Socket descriptor for client */
struct sockaddr_in httpServAddr; /* Local address */
struct sockaddr_in httpClntAddr; /* Client address */
unsigned short httpServPort; /* Server port */
unsigned int clntLen; /* Length of client address data structure */
if (argc != 2) /* Test for correct number of arguments */
{
fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
exit(1);
}
httpServPort = atoi(argv[1]); /* First arg: local port */
/* Create socket for incoming connections */
if ((servSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
DieWithError("socket() failed"); 


/* Construct local address structure */
memset(&httpServAddr, 0, sizeof(httpServAddr)); /* Zero out structure */
httpServAddr.sin_family = AF_INET; /* Internet address family */
httpServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
httpServAddr.sin_port = htons(httpServPort); /* Local port */
/* Bind to the local address */
if (bind (servSock, (struct sockaddr *) &httpServAddr, sizeof(httpServAddr))<0)
DieWithError("bind() failed");
/* Mark the socket so it will listen for incoming connections */
if (listen (servSock, MAXPENDING) < 0)
DieWithError("listen() failed");

for (;;) /* Run forever */
{
/* Set the size of the in-out parameter */
clntLen = sizeof(httpClntAddr); /* Wait for a client to connect */
if ((clntSock = accept (servSock, (struct sockaddr *) &httpClntAddr, &clntLen)) < 0)
DieWithError("accept() failed");
/* clntSock is connected to a client! */
printf("Handling client %s\n", inet_ntoa(httpClntAddr.sin_addr));
HandleTCPClient(clntSock);
}

} 
void DieWithError(char *errorMessage)
{

	printf(errorMessage);

}
void HandleTCPClient(int clntSocket) /* TCP client handling function */
{
	char buf[BUFFER_SIZE];
	int read;
	if((read = recv(clntSocket, buf, BUFFER_SIZE, 0)) == -1) DieWithError("Client read failed\n");

	/*Extracts the different parts of the html request*/
	char* token = strtok(buf, " ");
	/*The type of request being made*/
	char requestType[5];
	/*The path for the specified file*/
	char filePath[MAX_FILEPATH_LENGTH];
	/*Position of the token in the request*/
	int pos = 0;


	while(token !=NULL){
		char temp[MAX_FILEPATH_LENGTH];/*temporary buffer*/
		strcpy(temp, token);
		if(pos == 0){
			strcpy(requestType, temp);
			pos++;
		}
		else if(pos == 1){
			strcpy(filePath, temp);
			pos++;
		}
		
		//printf("Token: %s\n", temp);
		token = strtok(NULL, " ");
	}

	//Is whether or not the reuqest is a GET request
	int getRequest = strcmp(requestType, "GET");
	//printf("Request: %s\tFile path: %s\n", requestType, filePath);
	
	FILE *toSend = fopen(filePath, "r");
	if(getRequest==0){
		if(toSend!=NULL){
			if(send(clntSocket, "HTTP/1.1 200 OK\r\n\r\n", strlen("HTTP/1.1 200 OK\r\n\r\n"),0)==-1) DieWithError("send() header failed");
		}
		else{
			if(send(clntSocket, "HTTP/1.1 404 Not Found\r\n\r\n", strlen("HTTP/1.1 404 Not Found\r\n\r\n"),0)==-1) DieWithError("send() header failed");
		}	
	}
	else{
		if(send(clntSocket, "HTTP/1.1 400 Bad Request\r\n\r\n", strlen("HTTP/1.1 400 Bad Request\r\n\r\n"),0)==-1) DieWithError("send() header failed");
	}
	
	//the file data being transmitted during the html request
	char data[RESPONSE_CHUNCK_SIZE+1];
	
	int times = 0; /*created for debugging*/


	if(getRequest == 0 && toSend!=NULL){
		int read; /*Number of bytes read*/
		FILE* end = fopen(filePath, "r");/*Points to the end of the file*/
		fseek(toSend, 0, SEEK_SET);
		fseek(end, 0, SEEK_END);

		while((read = fread(data, 1, RESPONSE_CHUNCK_SIZE-1, toSend)>0)){
			printf("sending: %s\n", data);
			times++;
			if(send(clntSocket, data, strlen(data)+1,0)==-1) DieWithError("send() body failed");
			memset(data, 0,RESPONSE_CHUNCK_SIZE); /*Clears memory allocated to receiving buffer*/
		}
		//if(send(clntSocket, "\r\n", 2,0)==-1) DieWithError("send() ending failed");
		printf("read done\n");
		if(read < 0) DieWithError("fread() failed");
		
	}

	close(clntSocket);
	printf("Client socket closed\n");
	
	
}
/* NOT REACHED */
