#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define null NULL
#define MIN_CHILD 8 
#define MAX_CHILD 13
#define EXITCODEMULTIPLIER 256
/*
This is the main function that is executed when the program first runs
It reads in a seed from a file callled seed.txt and uses
the seed in order to be able to run the srand function 
It then uses the rand function to generate the number of maximum children allowed
It then creates an array equal to the size of the children
It initializes a counter to keep track of the number of children
It then continues to fork and call upon the randomly generated numbers
from the array in order to determine how long the child process will
wait until it terminates 
*/
int main(int argc, char * argv[]){
    FILE* seedFile = fopen("seed.txt", "r"); //Opens file named seed.txt
    int *exitCode = malloc(sizeof(int)); //The exit code of the child 
    int status = 0; //The status of the parent process
    char buffer[BUFFER_SIZE]; //Used to read in the stored seed value
    if(seedFile!=null){
        int childPID = -1; //The PID of the child process (stored by parent)
        int myPID = -1; //The PID of the current process
        fgets(buffer, BUFFER_SIZE, seedFile); //Reading in the contents of the seed file 
        printf("Read seed value: %s\n\n", buffer);
        int seed = atoi(buffer); //Converts the read in string to an integer
        printf("Read seed value (converted to integer): %d\n", seed);
        srand(seed); //Setting the seed for randomizer
        fclose(seedFile); // Closes the file containing the seed
        int children = (rand()%(MAX_CHILD - MIN_CHILD + 1)+MIN_CHILD); //Determines the number of children
        printf("Random Child Count: %d\n", children);
        printf("I'm feeling prolific!\n");
        int numbers[MAX_CHILD]; //stores the random numbers for each child
        for(int i = 0; i < children; i++){
            numbers[i] = rand();//sets the random numbers for each child
        }
        for(int child = 0; child < children; child++){
            childPID = fork();
            if(childPID > 0){
                printf("[Parent]: I am waiting for PID %d to finish \n", childPID);
            }
            else if(childPID == 0){
                myPID = getpid(); //Getting PID of child
                int myRandom = numbers[child]; //Used for the child to determine wait time and exit code
                printf("\t[Child, PID: %d]: I am the child and I will wait %d seconds and exit with code %d\n", myPID, ((myRandom%3)+1), ((myRandom%50)+1));
                sleep((myRandom%3)+1); //The time in which the child is pausing
                printf("\t[Child, PID: %d]: Now exiting...\n", myPID);
                exit((myRandom%50)+1);
            }
            childPID = waitpid(childPID, exitCode, 0);
            printf("[Parent]: Child %d finished with status code %d. Onward!\n", childPID, *exitCode/EXITCODEMULTIPLIER);
        }
        
    }
    else{
        //If the file is not found will return -1 and print message
        printf("Seed file not found\n");
        status =  -1;
    }
    free(exitCode); //free heap memory
    return status;
}