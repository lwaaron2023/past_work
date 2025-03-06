#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <sys/types.h>
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include <netdb.h>
#include <sys/time.h>
#define RESPONSE_CHUNCK_SIZE 2048
#define MAX_FILEPATH_LENGTH 128
#define MAX_URL_LENGTH 128
void DieWithError(char *errorMessage); /* Error handling function */ 
void getParts(char *wholeUrl, char *servUrl, char filePath[]); /*divides up commandline input into workable parts*/

int main(int argc, char *argv[])
{
int sock; /* Socket descriptor */
char *httpPort; /*HTTP server port as a char pointer (only used in getaddrinfo())*/
char *wholeUrl; /*The entire url that has been selected*/
char servUrl[MAX_URL_LENGTH]; /*The host part of a URL*/
char filePath[MAX_FILEPATH_LENGTH]; /*The file part of a URL*/
int rtt; /* Whether or not a round trip time is calculated*/
unsigned long start; /* The starting time for the rount trip time */
unsigned long end; /* The ending time for the round trip time */
struct addrinfo basicInfo; /* The basic info of the type of server the request will be sent to*/
struct addrinfo *serverInfo; /* The information pertaining to the specific*/

memset(servUrl, 0,MAX_URL_LENGTH); /*Clears memory allocated to servUrl*/
memset(filePath,0,MAX_FILEPATH_LENGTH);/*Clears memory allocated to filePath*/


/* Test for correct number of arguments */
if(argc == 3){
        rtt = 0; /*tells the code not to calculate RTT*/
        wholeUrl = argv[1]; /*sets the server url to be the second argument*/
        httpPort = argv[2]; /*Uses given port*/
}

else if(argc == 4 && strcmp(argv[1],"-p")==0){
        rtt = 1; /*tells the code to calculate RTT*/
        wholeUrl = argv[2]; /*sets the server url to be the second argument*/
        httpPort = argv[3]; /*Uses given port*/
}
else{
        fprintf(stderr, "Usage: %s [<-options>] <Server_url> <Port_number>\n",
        argv[0]);
        exit(1);
}

getParts(wholeUrl, servUrl, filePath);

//printf("Hostname: %s,%lu Filepath: %s\n", servUrl, strlen(servUrl), filePath);


memset(&basicInfo, 0, sizeof basicInfo);
basicInfo.ai_family = AF_INET;
basicInfo.ai_socktype = SOCK_STREAM;
basicInfo.ai_flags = AI_PASSIVE;


//exit(0);
if((getaddrinfo(servUrl, httpPort, &basicInfo, &serverInfo)) != 0) 
DieWithError("getaddrinfo() failed\n");

/* Create a reliable, stream socket using TCP */
if((sock = socket (serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0)
DieWithError("socket() failed\n");

struct timeval ct;
/*Calculates start time for RTT*/
if(rtt == 1){
        if(gettimeofday(&ct, NULL) == 0)
        start = (1000*ct.tv_sec) + (ct.tv_usec/1000);
        else{
                DieWithError("gettimeofday() failed");
        }
}
/* Connects to the server to begin sending requests*/
if(connect(sock, serverInfo->ai_addr, serverInfo -> ai_addrlen)!=0){
        printf("connect() failed\n");
}

/*Calculates end time for RTT*/
if(rtt == 1){
        if(gettimeofday(&ct, NULL) == 0)
        end = (1000*ct.tv_sec) +(ct.tv_usec/1000);
        else{
                DieWithError("gettimeofday() failed");
        }
}

//The message to be sent
char message[256] ="GET";
/*Creating the message*/
strcat(message, " ");
strcat(message, filePath);
strcat(message, " ");
strcat(message, "HTTP/1.1");
strcat(message, "\r\n");
strcat(message, "\r\n");


//printf("Message %s With Length%lu\n", message, strlen(message));

/*Sends HTTP Request*/
if(send(sock, message, strlen(message), 0)==-1) {DieWithError("send() failed");}
/*Reads HTTP Response*/
char buf[RESPONSE_CHUNCK_SIZE];
int length;
int times = 0;

memset(buf, 0,RESPONSE_CHUNCK_SIZE); /*Clears memory allocated to receiving buffer*/
int isData = 0; /*Represents whether or not the data part of the response has been reached*/

while((length = recv(sock, buf, RESPONSE_CHUNCK_SIZE, 0))>0){
        times++;
        printf("%s\n", buf);
        //printf("%d:\tLength: %d\n", times, length);
        //printf("%lu\n",strrchr(buf, 0));
        if(strstr(buf,"</html>")!=NULL){
                isData = 1;
        }
        if(isData == 1 && strrchr(buf,'\r')-buf == length-2 && strrchr(buf,'\n')-buf == length-1 && strrchr(buf,0)-buf == length){ /*Checks to see if message ends in the sequence {\r,\n,0} character*/        
                break;
        }
        memset(buf, 0,RESPONSE_CHUNCK_SIZE); /*Clears memory allocated to receiving buffer*/
                
        

        
}
if(length ==-1){ 
        DieWithError("recv() failed");

}

//printf("done receiving\n");
//printf("Size of Data received: %d\n", length);

/*
char* token = strtok(buf, "<\\html>");
while(token != NULL){
        printf("%s\n", token);
        token = strtok(NULL, "<\\html>");
}
*/


/*Prints RTT*/
if(rtt == 1){
        printf("RTT is %lu milliseconds\n",end-start);
}



freeaddrinfo(serverInfo);
close (sock);
exit(0);
}

void DieWithError(char *errorMessage)
{
        printf(errorMessage);
        printf("\n");
        exit(-1);
} 

void getParts(char *wholeUrl, char* servUrl, char *filePath){
        //printf("Function call\n URL: %s\n", wholeUrl);
        char* token = strtok(wholeUrl, "/");
        int i = 0;
       
        
        while(token != NULL){
                char *temp = token;
                if(i == 0){
                        if(strlen(temp) < MAX_URL_LENGTH){
                                strcpy(servUrl, temp);
                        }
                        else{
                                DieWithError("URL too long");
                        }
                }
                else{
                        if(strlen(filePath)+strlen(temp)+1 < MAX_FILEPATH_LENGTH){
                                strcat(filePath, "/");
                                strcat(filePath, temp);
                        }
                        else{
                                DieWithError("Filepath too long");
                        }
                }
                //printf("%d Token: %s\n", i, token);
                token = strtok(NULL, "/");
                i++;
                
        }
        if(i == 1){
                strcpy(filePath, "/index.html");
        }
}
