Important Information (please read contents of file BEFORE running any code):


1) Compiling:

There are two ways to compile the code so that it runs correctly. It can either be done via the makefile or by mannually compiling each file. 
Please note that in order for the makefile to work, the current file hierachy must be kept.

a) For the makefile:

Run the command: 

make 

To compile simulation.c into the simulation

Run the command:

make clean

To remove the executables simulation


b) For mannual compiling:

i) Run the command for simulation:

gcc -Wall -pthread simulation.c -o simulation


2) Running the programs:

a) Running simulation:

Type the command: 

./simulation


Answer to Questions

1) My solution avoids thread starvation by using a variable to store the last team as well as putting a maximum number of games that each player can play. 
Using the team variable I make sure that if the last sport was same as the sport that is about to enter the field, it forfeits its spot on the filed to the next groups of players.
Therefore, it guarentees that at least one different sport will be played every so often because it makes it harder for back to back games to occur.
In order for back to back taking of the field to occur either only one sport is still active (i.e. no one waiting for semaphore) or the same sport must be give up its spot, 
then another group of players must be chosen and give up their spot, then the original group of players (those that gave up their spot first) are chosen to take the field. 
Given the complexity of this sequence, the likihood of the field being used by the same sport two times in a row is lower. Adding a maximum number of games also helps with 
avoiding thread starvation because if two sports are taking up too much time on the field, they eventually give up the field entirely to the other sport, guarenting that each sport will get 
the field at least 6 times (in the case of football or baseball exactly 6 times).

2) The types of test cases that would be the most effectivee for identifying synchronization issues would be testing the functions that allow players to queue,
in order to make sure that a group of players cannot take the field without sufficient numbers. Additionally, another test case that would be usefull is testing
behavior when there are multiple teams trying to take the field to make sure that only one team can take the field. The output.txt file that the program writes to
shows these test cases because it shows that teams can only take the field when ready (i.e. all there are enough baseball players in order to take the field). Additionally,
in the case of the rugby players, the test case shows that there are not too many players trying to take the field at a given time by the fact that the starting value
is less than or equal to 30. Additionally, the number of players who leave the field is less than or equal to the number that are substituted onto the field.