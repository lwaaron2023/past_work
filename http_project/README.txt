PLEASE NOTE:

1) Code from the files http_server.c, http_client.c, and makefile are original code

2) The contents of other files were included as part of the starting code for the class project


Important Information:


1) Compiling:

There are two ways to compile the code so that it runs correctly. It can either be done via the makefile or by mannually compiling each file. 
Please note that in order for the makefile to work, the current file hierachy must be kept.

a) For the makefile:

Run the command: 

make all

To compile http_client.c into http_client and http_server.c into http_server.

Run the command:

make clean

To remove the the http_client and http_server executable files.

b) For mannual compiling:

i) Go to the directory in which http_server.c exists (should be under serverFIles), and run the command:

gcc -Wall http_server.c -o http_server

To compile http_server.c into http_server.

ii) Go to the directory in which http_client.c exists (should be under clientFIles), and run the command:

gcc -Wall http_client.c -o http_client

To compile http_client.c into http_client.


2) Running the program:

a) Running http_client:

The command to run http_client once compiled should be:

./http_client hostname/filePath portNumber

or

./http_client -p hostname/filePath portNumber

Where:

hostname is the name of the location that houses the file (for example the specified linux server or a server such as www.google.com).

/filePath is the ABSOLUTE FILE PATH of the requested file; if there is no filePath specified the code will look for /index.html.

portNumber is the port number of the location that houses the file is listening to (typically 80, but can be varried for the http_server).

-p is an indicator that the RTT should be printed after the contents of the respond are printed.

b) Running http_server:

The command to run http_server once compiled should be:

./http_server portNumber

Where:

portNumber is the port number that the server will be listening to.