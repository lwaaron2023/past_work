#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 256
#define null NULL 
#define BASEBALL_PLAYERS 36
#define RUGBY_PLAYERS 60
#define FOOTBALL_PLAYERS 44
#define BASEBALL_TEAM_SIZE 18
#define FOOTBALL_TEAM_SIZE 22
#define NUM_BASEBALL_TEAMS 2
#define NUM_FOOTBAlL_TEAMS 2
#define NUM_RUGBY_PAIRS 30
#define NO 0
#define YES 1
#define BASEBALL 2
#define FOOTBALL 3
#define RUGBY 4
#define WAIT_TIME ((rand()%2)+1)
#define GAME_TIME ((rand()%MAX_GAME_TIME)+1)
#define MAX_GAME_TIME 6


//This stores all information for 18 baseball players
struct BaseballTeam{
    pthread_t players[BASEBALL_TEAM_SIZE];
    int numReadyPlayers;
    int playerTimes[BASEBALL_TEAM_SIZE];
    sem_t hasField;
    pthread_mutex_t lock;
};
//This stores all information for 22 football players
struct FootballTeam{
    pthread_t players[FOOTBALL_TEAM_SIZE];
    int numReadyPlayers;
    int playerTimes[FOOTBALL_TEAM_SIZE];
    sem_t hasField;
    pthread_mutex_t lock;
};
//This stores all information for 2 rugby players
struct RugbyPair{
    pthread_t players[2];
    int numReadyPlayers;
    int playerTimes[2];
    sem_t hasField;
    pthread_mutex_t lock;
};

typedef struct BaseballTeam baseteam;
typedef struct FootballTeam footteam;
typedef struct RugbyPair rugpair;

//This stores information for any type of player
struct Player{
    int playerNum;
    int teamNum;
};


typedef struct Player plyr;

void* playBaseball(void* num);
void* playFootball(void* num);
void* playRugby(void* num);
int readyUpBaseball(int teamNum, int playerNum);
int readyUpFootball(int teamNum, int playerNum);
int readyUpRugby(int pairNum, int playerNum);
int enterFieldBaseball(int teamNum, int playerNum);
int enterFieldFootball(int teamNum, int playerNum);
int enterFieldRugby(int pairNum, int playerNum);
void* scheduleRugby(void * num);

FILE *output; // the output file 
baseteam bteam[NUM_BASEBALL_TEAMS]; // stores all the baseball teams 
footteam fteam[NUM_FOOTBAlL_TEAMS]; // stores all the football teams
rugpair rpair[NUM_RUGBY_PAIRS]; // stores all the rugby pairs
int waitingRugbyPairs = 0; // used to indicate the number of rugby pairs wiating to use the field
int playingRugbyPairs = 0; // used to indicate the number of rugby pairs on the field
int activeRugbyPlayers = 0; // used to indicate the number of rugby pairs still playing at all
pthread_mutex_t rugbyLine; // used to control access to waiting line
pthread_mutex_t rugbyField; // used to control access to the rugby players on field
pthread_mutex_t activeRugby; // used to control access to the number of active rugby players
sem_t field; // determines who gets the field
sem_t substitutions; // determines which rugby pairs are on the field when it is rugby's turn
pthread_t rugbyScheduler; // schedules the rugby matches so that it can be done more effectively
int lGame; //represents the last game to play
pthread_mutex_t lastField; 


/*
This functions initializes all the player threads
It waits for the player threads to terminate before exiting the program
*/
int main(int argc, char * argv[]){

    FILE* seedFile = fopen("seed.txt", "r"); //Opens file named seed.txt
    output = fopen("output.txt", "w");
    if(seedFile!=null && output != null){
        char buffer[BUFFER_SIZE]; //Used to read in the stored seed value
        fgets(buffer, BUFFER_SIZE, seedFile); //Reading in the contents of the seed file 
        fprintf(output, "Read seed value: %s\n", buffer);
        int seed = atoi(buffer); //Converts the read in string to an integer
        fprintf(output, "Read seed value (converted to integer): %d\n\n", seed);
        srand(seed); //Setting the seed for randomizer
        fclose(seedFile); // Closes the file containing the seed
        fflush(output);
        //This section is initializing global semaphores and mutexs
        sem_init(&field, 0, 1);
        sem_init(&substitutions, 0, 0);
        pthread_mutex_init(&rugbyLine, null);
        pthread_mutex_init(&rugbyField, null);
        pthread_mutex_init(&activeRugby, null);
        pthread_mutex_init(&lastField, null);
        //fill each baseball team with players
        for(int i = 0; i < NUM_BASEBALL_TEAMS; i++){
            pthread_mutex_init(&bteam[i].lock, null);
            sem_init(&bteam[i].hasField, 0, 0);
            for(int j = 0; j < BASEBALL_TEAM_SIZE; j++){
                plyr *player = malloc(sizeof(plyr));
                player->playerNum = j;
                player->teamNum = i;
                pthread_create(&(bteam[i].players[j]), null, playBaseball, (void*)player);
            }
        }
        //fill each football team with players
        for(int i = 0; i < NUM_FOOTBAlL_TEAMS; i++){
            pthread_mutex_init(&fteam[i].lock, null);
            sem_init(&fteam[i].hasField, 0, 0);
            for(int j = 0; j < FOOTBALL_TEAM_SIZE; j++){
                plyr *player = malloc(sizeof(plyr));
                player->playerNum = j;
                player->teamNum = i;
                pthread_create(&(fteam[i].players[j]), null, playFootball, (void*)player);
            }

        }
        //fill each pair of rugby players with players
        for(int i = 0; i < NUM_RUGBY_PAIRS; i++){
            pthread_mutex_init(&rpair[i].lock, null);
            sem_init(&rpair[i].hasField, 0, 0);
            plyr *player1 = malloc(sizeof(plyr));
            player1->playerNum = 0;
            player1->teamNum = i;
            plyr *player2 = malloc(sizeof(plyr));
            player2->playerNum = 1;
            player2->teamNum = i;
            pthread_create(&(rpair[i].players[0]), null, playRugby, (void*)player1);
            pthread_create(&(rpair[i].players[1]), null, playRugby, (void*)player2);
        }
        pthread_create(&rugbyScheduler, null, scheduleRugby, null);
        
        //waits for baseball threads to be done
        for(int i = 0; i < NUM_BASEBALL_TEAMS; i++){
            for(int j = 0; j < BASEBALL_TEAM_SIZE; j++){
                pthread_join(bteam[i].players[j], null);
            }
        }
        fprintf(output, "\nBASEBALL IS NO LONGER BEING PLAYED\n");
        fflush(output);
        //waits for football threads to be done
        for(int i = 0; i < NUM_FOOTBAlL_TEAMS; i++){
            for(int j = 0; j < FOOTBALL_TEAM_SIZE; j++){
                pthread_join(fteam[i].players[j], null);
            }

        }
        fprintf(output, "\nFOOTBALL IS NO LONGER BEING PLAYED\n");
        fflush(output);
        //waits for rugby threads to be done
        pthread_join(rugbyScheduler, null);
        fprintf(output, "\nRUGBY IS NO LONGER BEING PLAYED\n");
        fflush(output);
        fclose(output);
    }
    else{
        //If the file is not found will return -1 and print message
        printf("Seed file not found\n");
        exit(-1);
    }
    return 0;
}

void* playBaseball(void* num){
    int teamNum = ((plyr*)num)->teamNum; 
    int playerNum = ((plyr*)num)->playerNum;
    int timesPlayed = 0; //number of times a baseball player has waited
    free(num);
    while(timesPlayed < 3){
        sleep(WAIT_TIME);
        int rtr = readyUpBaseball(teamNum, playerNum);
        if(rtr == 0){
            //if the team is not all ready then it waits here until allowed to procede
            fprintf(output, "BASEBALL: Team %d, player is ready to play %d, has played %d times\n", teamNum, playerNum, timesPlayed);
            fflush(output);
            sem_wait(&bteam[teamNum].hasField);
            sleep(bteam[teamNum].playerTimes[0]);
            timesPlayed++;
        }
        else{
            //if the team is all ready then it will continue to try and schedule a game
            fprintf(output, "BASEBALL: Team %d, player %d, has played %d times, TEAM IS READY TO PLAY\n", teamNum, playerNum, timesPlayed);
            fflush(output);
            enterFieldBaseball(teamNum, playerNum);
            timesPlayed++;
        }
    }    
    return NULL;
}

void* playFootball(void* num){
    int teamNum = ((plyr*)num)->teamNum;
    int playerNum = ((plyr*)num)->playerNum;
    int timesPlayed = 0;
    free(num);
    while(timesPlayed < 3){
        sleep(WAIT_TIME);
        int rtr = readyUpFootball(teamNum, playerNum);
        if(rtr == 0){
            //if the team is not all ready then it waits here until allowed to procede
            fprintf(output, "FOOTBALL: Team %d, player %d is ready to play, has played %d times\n", teamNum, playerNum, timesPlayed);
            fflush(output);
            sem_wait(&fteam[teamNum].hasField);
            sleep(fteam[teamNum].playerTimes[0]);
            timesPlayed++;
        }
        else{
            //if the team is all ready then it will continue to try and schedule a game
            fprintf(output, "FOOTBALL: Team %d, player %d, has played %d times, TEAM READY TO PLAY\n", teamNum, playerNum, timesPlayed);
            fflush(output);
            enterFieldFootball(teamNum, playerNum);
            timesPlayed++;
        }
    }
    return NULL;
}

void* playRugby(void* num){
    int pairNum = ((plyr*)num)->teamNum;
    int playerNum = ((plyr*)num)->playerNum;
    int timesPlayed = 0;
    free(num);
    pthread_mutex_lock(&activeRugby);
    //used to communicate with the scheduler, telling it an additional player is playing
    activeRugbyPlayers++;
    pthread_mutex_unlock(&activeRugby);
    while(timesPlayed < 3){
        sleep(WAIT_TIME);
        
        int rtr = readyUpRugby(pairNum, playerNum);
        if(rtr == 0){
            //if the pair is not all ready then it waits here until allowed to procede
            fprintf(output, "RUGBY: pair %d, player %d is ready to play, has played %d times\n", pairNum, playerNum, timesPlayed);
            fflush(output);
            sem_wait(&rpair[pairNum].hasField);
            sleep(rpair[pairNum].playerTimes[0]);
            timesPlayed++;
        }
        else{
            //if the pair is all ready then it will continue to try and get in line to be put on the field by the scheduler
            fprintf(output, "RUGBY: pair %d, player %d, has played %d times, PAIR IS READY TO PLAY\n", pairNum, playerNum, timesPlayed);
            fflush(output);
            enterFieldRugby(pairNum, playerNum);
            timesPlayed++;
        }
    }
    pthread_mutex_lock(&activeRugby);
    //used to communicate with the scheduler, telling it a player is no longer playing
    activeRugbyPlayers--;
    pthread_mutex_unlock(&activeRugby);
    return NULL;
}

    
int readyUpBaseball(int teamNum, int playerNum){
    int rtn = 0;
    pthread_mutex_lock(&(bteam[teamNum].lock));
    //tells the team that another player is ready
    bteam[teamNum].numReadyPlayers++;
    bteam[teamNum].playerTimes[playerNum] = GAME_TIME;
    if(bteam[teamNum].numReadyPlayers % BASEBALL_TEAM_SIZE == 0){
        //if all players are ready it calculates the average suggested time and stores it in the first suggested wait time
        rtn = 1;
        int temp = bteam[teamNum].playerTimes[0];
        for(int i = 1; i < BASEBALL_TEAM_SIZE; i++){
            temp += bteam[teamNum].playerTimes[i];
        }
        temp/=BASEBALL_TEAM_SIZE;
        bteam[teamNum].playerTimes[0] = temp;
    }
    pthread_mutex_unlock(&(bteam[teamNum].lock));
    return rtn;
}


int readyUpFootball(int teamNum, int playerNum){
    int rtn = 0;
    pthread_mutex_lock(&(fteam[teamNum].lock));
    //tells the team that another player is ready
    fteam[teamNum].numReadyPlayers++;
    fteam[teamNum].playerTimes[playerNum] = GAME_TIME;
    if(fteam[teamNum].numReadyPlayers % FOOTBALL_TEAM_SIZE == 0){
        //if all players are ready it calculates the average suggested time and stores it in the first suggested wait time
        rtn = 1;
        int temp = fteam[teamNum].playerTimes[0];
        for(int i = 1; i < FOOTBALL_TEAM_SIZE; i++){
            temp += fteam[teamNum].playerTimes[i];
        }
        temp/=FOOTBALL_TEAM_SIZE;
        fteam[teamNum].playerTimes[0] = temp;
    }
    pthread_mutex_unlock(&(fteam[teamNum].lock));
    return rtn;
}


int readyUpRugby(int pairNum, int playerNum){
    int rtn = 0;
    pthread_mutex_lock(&(rpair[pairNum].lock));
    //tells the pair that another player is ready
    rpair[pairNum].numReadyPlayers++;
    rpair[pairNum].playerTimes[playerNum] = GAME_TIME;
    if(rpair[pairNum].numReadyPlayers % 2 == 0){
        //if all players are ready it calculates the average suggested time and stores it in the first suggested wait time
        rpair[pairNum].playerTimes[0] = ( rpair[pairNum].playerTimes[0]+ rpair[pairNum].playerTimes[1])/2;
        rtn = 1;
    }
    pthread_mutex_unlock(&(rpair[pairNum].lock));
    return rtn;
}


int enterFieldBaseball(int teamNum, int playerNum){
    sem_wait(&field);
    pthread_mutex_lock(&lastField);
    //to reduce chance of back to back baseball
    if(lGame == BASEBALL){
        pthread_mutex_unlock(&lastField);
        sem_post(&field);
        sleep(1);
        sem_wait(&field);
    }
    fprintf(output, "\nBASEBALL HAS THE FIELD: team %d is playing\n The game should last %d seconds\n\n", teamNum, bteam[teamNum].playerTimes[0]);
    fflush(output);
    //once a team has a field, the player who went on first notifies the other players by increasing the semaphore so that those player's operations will cease to block
    int temp = bteam[teamNum].playerTimes[0];
    for(int i = 1; i < BASEBALL_PLAYERS; i++){
        sem_post(&bteam[teamNum].hasField);
    }
    sleep(temp);
    lGame = BASEBALL;
    pthread_mutex_unlock(&lastField);
    sem_post(&field);
    return 0;
}
int enterFieldFootball(int teamNum, int playerNum){
    sem_wait(&field);
    pthread_mutex_lock(&lastField);
    //to reduce chance of back to back football
    if(lGame == FOOTBALL){
        pthread_mutex_unlock(&lastField);
        sem_post(&field);
        sleep(1);
        sem_wait(&field);
    }
    fprintf(output, "\nFOOTBALL HAS THE FIELD: team %d is playing\n The game should last %d seconds\n\n", teamNum, fteam[teamNum].playerTimes[0]);
    fflush(output);
    //once a team has a field, the player who went on first notifies the other players by increasing the semaphore so that those player's operations will cease to block
    int temp = fteam[teamNum].playerTimes[0];
    for(int i = 1; i < FOOTBALL_PLAYERS; i++){
        sem_post(&fteam[teamNum].hasField);
    }
    sleep(temp);
    lGame = FOOTBALL;
    pthread_mutex_unlock(&lastField);
    sem_post(&field);
    return 0;
}
int enterFieldRugby(int pairNum, int playerNum){
    pthread_mutex_lock(&rugbyLine);
    //tells the scheduler another pair is waiting to be put on the field
    waitingRugbyPairs++;
    pthread_mutex_unlock(&rugbyLine);
    sem_wait(&substitutions);
    //once the pair has bee approved on the field the other partner is notified
    sem_post(&rpair[pairNum].hasField);
    sleep(rpair[pairNum].playerTimes[0]);
    pthread_mutex_lock(&rugbyField);
    //tells the scheduler a pair has left the field
    playingRugbyPairs--;
    pthread_mutex_unlock(&rugbyField);
    return 0;
}

void* scheduleRugby(void * num){
    while(1){
        pthread_mutex_lock(&activeRugby);
        if(activeRugbyPlayers <= 0){
            //if there are no active players then break the loop and terminate
            break;
        }
        int temp;
        pthread_mutex_lock(&rugbyLine);
        //checks the number of waiting pairs
        temp = waitingRugbyPairs;
        pthread_mutex_unlock(&rugbyLine);
        pthread_mutex_unlock(&activeRugby);
        if(temp > 0){
            //if at least one pair is waiting gets in line for the field
            sem_wait(&field);
            pthread_mutex_lock(&lastField);
            //to reduce chance of back to back rugby
            if(lGame == RUGBY){
                pthread_mutex_unlock(&lastField);
                sem_post(&field);
                sleep(1);
                sem_wait(&field);
            }
            char buffer1[BUFFER_SIZE/4]; //Used to store the string for rugby
            char buffer2[BUFFER_SIZE/4]; //Used to store the string for rugby
            char buffer3[BUFFER_SIZE]; //Used to store the string for rugby
            sprintf(buffer1, "\nRUGBY HAS THE FIELD: The game should last %d seconds\n", (3*MAX_GAME_TIME)/2);
            pthread_mutex_lock(&rugbyField);
            int initialPairs = 0; //keeps track of intial pairs
            playingRugbyPairs = 0; //keeps track of currently playing pairs, resets it to zero so that if the lock causes this to run (back to back game) it doesn't need to wait for the locks to reset it
            for(int i = 0; i < waitingRugbyPairs && i < (NUM_RUGBY_PAIRS/2); i++){
                    // wakes up pairs, either up to 15 or the number waiting in line at the time the field was secured for rugby, takes note of the added pairs
                    sem_post(&substitutions);
                    playingRugbyPairs++;
                    initialPairs++;
            }
            sprintf(buffer2, "The game has started with %d players\n", playingRugbyPairs * 2);
            pthread_mutex_unlock(&rugbyField);
            //waits for half the maximum possible game time to make sure that some pairs have left
            sleep(MAX_GAME_TIME/2);
            pthread_mutex_lock(&rugbyField);
            int subs = 0; //keeps track of the newly added players
            initialPairs-=playingRugbyPairs; //stores how many of the pairs are still left to play rugby
            for(int i = 0; i < waitingRugbyPairs && i < ((NUM_RUGBY_PAIRS/2)-playingRugbyPairs); i++){
                    // wakes up pairs, either enough to get 30 players or the number waiting in line at the time new pairs were allowed to take the field, takes note of the added pairs
                    sem_post(&substitutions);
                    playingRugbyPairs++;
                    subs++;
            }
            sprintf(buffer3, "There has been %d players who have left, and %d players replacing them as substitutes\n\n", initialPairs * 2, subs * 2);
            pthread_mutex_unlock(&rugbyField);
            // sleeps for max game time because all new threads should be completed by now since they can run for up to max game time
            sleep(MAX_GAME_TIME);
            fprintf(output, "%s%s%s", buffer1, buffer2, buffer3);
            lGame = RUGBY;
            pthread_mutex_unlock(&lastField);
            fflush(output);
            sem_post(&field);
        }
        else{
            //instead of securing field waits for arbitrary time, 3 seconds, because no pairs have gotten in line
            sleep(3);
        }
    }
    return null;
}



