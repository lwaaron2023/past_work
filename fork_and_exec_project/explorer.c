#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define null NULL 
#define MAX_CHILD 5
#define EXITCODEMULTIPLIER 256
#define LOCATIONS 6
/*
This is the main function that is executed when the program first runs
It reads in a seed from a file callled seed.txt and uses
the seed in order to be able to run the srand function 
It then for every child it will generate a random number
It will loop the following five times:
It will then generate a random number to determine which location to go to
It will change its directory to that location (if it fails it will exit after printing an error)
It will then create a fork() and run the ls -tr command
It will go back to the root directory
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
        printf("It's time to see the world/file system!\n");
        char *locations[LOCATIONS]; //creating a matrix of characters to store the various paths
        locations[0] = "/home"; //first location
        locations[1] = "/proc"; //second location
        locations[2] = "/proc/sys"; //third location
        locations[3] = "/usr"; //fourth location
        locations[4] = "/usr/bin"; //fifth location
        locations[5] = "/bin"; //sixth location
        int *code = malloc(sizeof(int)); //status code for child process
        for(int i = 0; i < MAX_CHILD; i++){
            int chosen = rand() % 6;
            
            //printf("CURRENT DIRECTORY: %s\n", buffer);
            //printf("ATTEMPTING TO CHANGE LOCATION\n");
            chdir(locations[chosen]);
            getcwd(buffer, BUFFER_SIZE);
            printf("Selection #%d: %s [%s]\n", i+1, locations[chosen], (strspn(locations[chosen], buffer) == strlen(locations[chosen]) ? "SUCCESS" : "FAILURE"));//uses the strspn() function in order to determine that the directory was changed successfully
            printf("Current Reported directory: %s\n", buffer);
            int temp = fork(); //Stores either the childPID for parent or 0 for child
            if(temp == 0){
                printf("\t[Child, PID: %d]: Executing ’ls -tr’ command...\n", getpid());
                char *command[3];
                command[0] = "ls";
                command[1] = "-tr";
                command[2] = NULL;
                int status = execvp("/bin/ls", command);
                exit(status);
            }
            printf("[Parent]: I am waiting for PID %d to finish.\n", temp);
            waitpid(temp, code, 0);
            if(*code == 0){
                printf("[Parent]: Child %d finished with status code 0. Onward!\n", temp);
            }
            else{
                printf("CHILD FAILED WITH CODE %d PROGRAM WILL TERMINATE\n", *code);
                status = -1;
                break;
            }
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