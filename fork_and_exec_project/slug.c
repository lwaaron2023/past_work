#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define null NULL 
#define MIN_WAIT 2
#define MAX_WAIT 6
#define COMMANDS 2
#define EXITCODEMULTIPLIER 256
#define LOCATIONS 6

const char file1[] = "seed_slug_1.txt"; //represents the filename of the first seed file
const char file2[] = "seed_slug_2.txt"; //represents the filename of the second seed file
const char file3[] = "seed_slug_3.txt"; //represents the filename of the third seed file
const char file4[] = "seed_slug_4.txt"; //represents the filename of the fourth seed file

/*
This is the main function that is executed when the program first runs
It takes in a command line argument indicating which seed file to read (1-4)
It reads in a seed from a file callled seed.txt and uses
the seed in order to be able to run the srand function 
It will then go to sleep for a random amount of time (2-6 seconds)
It will then either run the last -i -x or id --group command 
*/
int main(int argc, char * argv[]){
    FILE* seedFile = NULL; //Represents the file with seed, set to NULL to begin so that if wrong number of command lines arguments it will skip execution of most of the code
    if(argc == 2){
        int s = atoi(argv[1]); //The number selected for the file
        switch(s){
            case 1:
                seedFile = fopen(file1, "r"); //Opens file named seed_slug_1.txt
                break;

            case 2:
                seedFile = fopen(file2, "r"); //Opens file named seed_slug_2.txt
                break;

            case 3:
                seedFile = fopen(file3, "r"); //Opens file named seed_slug_3.txt
                break;


            case 4:
                seedFile = fopen(file4, "r"); //Opens file named seed_slug_4.txt
                break;


            default:
                printf("INCORRECT NUMBER INPUTTED FOR FILE SELECTION, MUST BE BETWEEN 1 and 4 INCLUSIVE\n");
                break;                
        }

    }
    else{
        printf("WRONG NUMBER OF ARGUMENTS TO RUN PROGRAM TYPE (%d): ./slug num\n", argc);
        
    }

    int status = 0; //The status of the parent process
    char buffer[BUFFER_SIZE]; //Used to read in the stored seed value
    
    if(seedFile!=null){
        int myPID = getpid(); //The PID of the current slug processes
        fgets(buffer, BUFFER_SIZE, seedFile); //Reading in the contents of the seed file 
        printf("[Slug PID: %d] Read seed value: %s\n\n", myPID, buffer);
        int seed = atoi(buffer); //Converts the read in string to an integer
        printf("[Slug PID: %d] Read seed value (converted to integer): %d\n", myPID, seed);
        srand(seed); //Setting the seed for randomizer
        fclose(seedFile); // Closes the file containing the seed
        int delay = (rand() % (MAX_WAIT-MIN_WAIT+1))+MIN_WAIT; //The amount of time the program will delay
        int flip = rand() % COMMANDS; //Which function will be executed
        printf("[Slug PID: %d] Delay time is %d seconds. Coin flip: %d\n", myPID, delay, flip);
        printf("[Slug PID: %d] I’ll get the job done. Eventually...\n", myPID);
        sleep(delay); //Causes processes to pause for delay seconds
        printf("[Slug PID: %d] Break time is over! I am running the ’%s’ command.\n", myPID, (flip == 1 ? "id --group" : "last -i -x"));
        if(flip == 1){
            char *command[3] = {"id","--group",NULL}; //Sets the arguments for the id program
            execvp("id", command); //Runs the id program with the given arguments
        }
        else{
            char *command[4] = {"last","-i","-x", NULL}; //Sets the arguments for the last program
            execvp("last", command); //Runs the last program with the given arguments
        }
    }
    else{
        //If the file is not found will return -1 and print message
        printf("Seed file not found\n");
        status =  -1;
    }
    return status;
}