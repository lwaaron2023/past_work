#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define null NULL
#define MIN_GEN 5 
#define MAX_GEN 12
#define EXITCODEMULTIPLIER 256

void createNextGeneration(int left);
/*
This is the main function that is executed when the program first runs
It reads in a seed from a file callled seed.txt and uses
the seed in order to be able to run the srand function 
It then uses the rand function to generate the number of generations to be allowed
It will then call the helper function below to create the generations
*/
int main(int argc, char * argv[]){
    FILE* seedFile = fopen("seed.txt", "r"); //Opens file named seed.txt
    int *exitCode = malloc(sizeof(int)); //The exit code of the child 
    int status = 0; //The status of the parent process
    char buffer[BUFFER_SIZE]; //Used to read in the stored seed value
    if(seedFile!=null){
        fgets(buffer, BUFFER_SIZE, seedFile); //Reading in the contents of the seed file 
        printf("Read seed value: %s\n\n", buffer);
        int seed = atoi(buffer); //Converts the read in string to an integer
        printf("Read seed value (converted to integer): %d\n", seed);
        srand(seed); //Setting the seed for randomizer
        fclose(seedFile); // Closes the file containing the seed
        int generations = (rand()%(MAX_GEN - MIN_GEN + 1)+MIN_GEN); //Determines the number of children
        printf("Random Descendant Count: %d\n", generations);
        printf("Time to meet the kids/grandkids/great grand kids/...\n");
        createNextGeneration(generations);
    }
    else{
        //If the file is not found will return -1 and print message
        printf("Seed file not found\n");
        status =  -1;
    }
    free(exitCode); //free heap memory
    return status;
}

/*
Takes in the number of generations left to create and if that is greater than
zero it will create a child process then the parent process will wait for the child process to repeat
The child process will then call this function with the counter having been decremented by 1
*/
void createNextGeneration(int left){
    int *code = malloc(sizeof(int)); //status code for child process
    if(left <= 0){ //Checks to see if this is the last generation
        exit(0); 
    }
    else{ //Otherwise creates a new process
        int temp = fork(); //Stores either the childPID for parent or 0 for child
        if(temp == 0){ //If the process is new it will run the function again
            printf("\t[Child, PID: %d]: I was called with descendant count=%d. I’ll have %d descendant(s).\n", getpid(), left, left-1);
            createNextGeneration(left-1);//This runs the function again but with a counter that has been decremented
        }
        printf("[Parent, PID: %d]: I am waiting for PID %d to finish.\n", getpid(), temp);
        waitpid(temp, code, 0);//This waits for the newly created child to complete before it completes
        printf("[Parent, PID: %d]: Child %d finished with status code %d. It’s now my turn to exit.\n", getpid(), temp, *code/EXITCODEMULTIPLIER);
        free(code); //frees memory attached to code pointer
        exit(left);   
    }
}