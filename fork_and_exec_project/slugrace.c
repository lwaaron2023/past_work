#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/signal.h>

#define timespec struct timespec
#define null NULL 
#define NUM_SLUGS 4
#define EXITCODEMULTIPLIER 256
#define LOCATIONS 6
#define BUFFER_SIZE 256
void d(int signum); // dummy function literally does absolutely nothing
/*
This is the main function that is executed when the program first runs
It creates an array to store the status of each slug (0 means slug has finished, its PID means it is still running)
It initiliazes one slug, pauses that slug, then intializes the next slug until all slugs are initializes and paused (via pause)
For each slug the fork() and execvp() functions are used
Then each slug is unpaused one after the other and the race begins
It uses a while loop and the waitpid to check every .33 seconds if a process has terminated
If it has then it will change the value in the slug array and calculate the ellapsed time
*/
int main(int argc, char * argv[]){
    
    timespec *current = malloc(sizeof(timespec)); //the current time of the system
    int slugs[NUM_SLUGS]; //represents whether or not a given slug is racing
    char *num[NUM_SLUGS]; //represents the number passes into the slug program
    int startTime;
    int s = clock_gettime(CLOCK_REALTIME, current); //gets the current time
    if(s == 0){
        startTime = current-> tv_sec; //gets the current second of the time (discards nanoseconds)
    }
    else{
        printf("FAILED TO GET TIME\n");
        exit(-1);
    }
    signal(SIGUSR1, d); //sets this signal to go to dummy function
    num[0] = "1";
    num[1] = "2";
    num[2] = "3";
    num[3] = "4";
    for(int i = 0; i < NUM_SLUGS; i++){
        int temp = fork(); //Stores either the childPID for parent or 0 for child
        if(temp == 0){
            int parentPID = getppid(); //getPID of parent used to communicate later that child is ready to call execvp()
            printf("\t[Child, PID: %d]: Executing ’./slug %d’ command...\n", getpid(), i+1);
            union sigval v; //needed for subsequent function, not used in other parts of code so values are arbitrary
            v.sival_int = 0;
            v.sival_ptr = null;
            sigqueue(parentPID, SIGUSR1, v); //using the first of two user defined linux signals
            char path[BUFFER_SIZE]; //the path for the slug file
            getcwd(path, BUFFER_SIZE); //the current directory
            strcat(path, "/slug"); //adds the file to execute
            char *command[3] = {"./slug", num[i], NULL}; //the command line arguments
            pause(); //waits for call from parent to continue to call execvp()
            execvp(path, command); //executes the slug program with the command line arguments
            printf("AN ERROR HAS OCCURRED\n");
            exit(-1);
        }
        else{
            slugs[i] = temp; 
            printf("[Parent]: I forked off child %d.\n", temp);
            pause(); //waits for child to print its first statement before it moves on to the next child
        }       
    }
    //This loop unpauses all of the slugs so that they can all begin the race relatively close to each other
    for(int i = 0; i < NUM_SLUGS; i++){
        union sigval v;
        v.sival_int = 0;
        v.sival_ptr = null;
        sigqueue(slugs[i], SIGUSR1, v);
    }
    char buffer[BUFFER_SIZE]; //a buffer for certain print statements
    int numRunning = NUM_SLUGS; //the number of slugs that are currently running
    int *status = malloc(sizeof(int)); //the exit status of child code
    //printf("PARENT FUNCTION HAS GOTTEN THIS FAR\n");
    while(numRunning > 0){
        int temp =  waitpid(0, status, WNOHANG); //Stores the childPID for the process that just finised, or 0 if none
        //since the value for slug cannpt be equal to 0, then this works 
        if(*status != 0 && temp >0){
            printf("CHILD PROCESS ENDED WITH ERROR\n");
            exit(-1);
        } 
        if(temp > 0){
            for(int i = 0; i < NUM_SLUGS; i++){
                if(slugs[i]==temp){  
                    numRunning--; //indicates one less slug is 
                    slugs[i] = -1; //indicates that slug i is no longer running
                    int s = clock_gettime(CLOCK_REALTIME, current); //gets the current time
                    if(s == 0){
                        int timeellapsed = current-> tv_sec - startTime; //gets the current second of the time (discards nanoseconds) and subtracts it from initial time
                        printf("Child %d has crossed the finish line! It took %d seconds\n", temp, timeellapsed);
                    }
                    else{
                        printf("FAILED TO GET TIME\n");
                        exit(-1);
                    }
                    break;
                }
            }
        }
        else{
            if(numRunning > 0){
                strcpy(buffer, "The race is ongoing. The following children are still racing: ");
                for(int i = 0; i < NUM_SLUGS; i++){
                    if(slugs[i] > 0){
                        char temp[BUFFER_SIZE];
                        sprintf(temp, "%d ", slugs[i]);
                        strcat(buffer, temp);
                    }
                }
                printf("%s\n", buffer);
                usleep(33000);
            }
        }
    }
    s = clock_gettime(CLOCK_REALTIME, current); //gets the current time
    if(s == 0){
        int timeellapsed = current-> tv_sec - startTime; //gets the current second of the time (discards nanoseconds) and subtracts it from initial time
        printf("The race is over! It took %d seconds\n", timeellapsed);
    }
    else{
        printf("FAILED TO GET TIME\n");
        exit(-1);
    }
    
    free(status);
    free(current);
    return 0;
}
// dummy function literally does absolutely nothing
void d(int signum){}



