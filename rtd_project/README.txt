PLEASE NOTE:

1) Code from the files student2.c, and makefile are original code

2) The contents of other files were included as part of the starting code for the class project 


Important Information:


1) Compiling:

There are two ways to compile the code so that it runs correctly. It can either be done via the makefile or by mannually compiling each file. 
Please note that in order for the makefile to work, the current file hierachy must be kept.

a) For the makefile:

Run the command: 

make 

To compile project2.c and student2.c into p2

Run the command:

make clean

To remove the the p2 executable files.

b) For mannual compiling:

Run the command:

gcc -g project2.c student2.c -o p2


2) Running the program:


The command to run p2 once compiled should be (you can also run without parameters if they are to be mannually entered at runtime):

./p2 NUMM LPP CPP OOP FRE TL R BD

Where:

NUMM is the number of messages to be simulated

LPP is the probability that packets will be lost (should set to between 0 and .2 for most cases; maximum allowed is 1.0 but not recommended)

CPP is the probability that packets will be corrupt (should be set to between 0 and .2 for most cases; maximum allowed is 1.0 but not recommended)

OOP is the probability that packets will be delivered out of order (should be set to between 0 and .2 for most cases; maximum allowed is 1.0 but not recommended)

FRE is the time it takes for new messages to arrive at A (set to 1000 for best results)

TL is the trace level (set to between 1 and 5)

R is whether or not the messages will be random (set 0 for no, 1 for yes)

BD is whether or not the messages should be sent bidirectionally (for ABP set 0 (no), for GBN set 1(yes))