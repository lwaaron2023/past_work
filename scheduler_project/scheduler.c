#include "queue.h"
#include <string.h>

void testQueue();
queue* getTasks(FILE* file, int sorted);
int roundRobin(queue *tasks, int interval);
int firstInFirstOut(queue* tasks, int sorted);

int main(int argc, char *argv[]){
    int ret = 0;
    if(argc != 4){
        printf("Usage: ./simulation <Policy name> <Job trace> <Time Slice>\n");
        ret = -1;
    }
    else{
        char *pName = argv[1];
        char *fName = argv[2];
        int tSlice  = atoi(argv[3]);
        char fPath[sizeof(argv[1])+BUFFER_SIZE] = ("/home/student/Desktop/Projects/project_3/");
        strcat(fPath, fName);
        //printf("CHOSEN VALUES: Policy Name: %s, Job trace: %s, Time slice: %d\n", pName, fPath, tSlice);
        FILE *file = fopen(fPath, "r");
        if(file!=null){
            queue* q; 
            if(strcmp("SJF", pName) == 0){ 
                //the queue is sortest so it can run a first come first server algorithm
                q = getTasks(file, 1);
                ret = firstInFirstOut(q, 1);
            }
            else if(strcmp("FIFO", pName) == 0){
                //the queue will run a first come first server
                q = getTasks(file, 0);
                ret = firstInFirstOut(q, 0);
            }
            else if(strcmp("RR", pName) == 0){
                //the queue will run a round robin algorithm
                q = getTasks(file, 0);
                if(tSlice > 0){
                    ret = roundRobin(q, tSlice);
                }
                else{
                    printf("INVALID TIMESLICE, MUST BE GREATER THAN ZERO/n");
                    ret = 1;
                }
                
            }
            else{
                printf("UNRECOGNIZED TASK, RECOGNIZED TASKS ARE: FIFO SJF RR\n");
                ret = -1;
            }
            //printQueue(q);
        }
        else{
            printf("FILE NOT FOUND\n");
            ret = -1;
        }
    }
    return ret;
}
/*
This function reads from the file and adds the task length to the list of tasks
*/
queue* getTasks(FILE* file, int sorted){
    //printf("SORTED VALUE %d\n", sorted);
    char buffer[MAX_TASK_DIGITS];
    queue* tasks = malloc(sizeof(queue));
    tasks-> first = null;
    tasks-> last = null;
    tasks-> size = 0;
    int num = 0; //current task added to list
    int index = 0; //current character added to buffer
    char c = 0;
    
    while(c!=EOF && index < MAX_TASK_DIGITS){
        //Checks to see if the current read character is not signifying the end of a file
        //or would make a number longer than MAX_TASK_DIGITS, or that the number of tasks
        //that have been read would become greater than MAX_TASK
        c = fgetc(file);
        
        if(c != EOF && (int) c > 32){
            //if the character is not a special character (i.e. space, line ending, etc)
            //add it to the buffer
            buffer[index] = c;
            index++;
        }
        else if(index > 0){
            //once encoutnering a speical character convert the buffer to an integer (from a string)
            //then store the string in the task queue, then augment the task id
            //reset the index in order to represent a new task length being discovered
            //reset the state of the buffer so that no leftover characters can be read
            job *j = malloc(sizeof(job));
            j->id = num;
            j->length = atoi(buffer);
            for(int i = 0; i < MAX_TASK_DIGITS; i ++){
                buffer[i] = 0;
            }
            if(sorted){
                addToOrderedQueue(tasks, createNode(j));
            }
            else{
                addToQueue(tasks, createNode(j));
            }
            index =  0;
            num++;
        }
    }
    return tasks;
}


int roundRobin(queue *tasks, int interval){
    printf("Execution trace with RR:\n");
    node *current = removeFirst(tasks);
    int time = 0;
    queue *fTasks = malloc(sizeof(queue)); //Queue of finished tasks
    fTasks->size = 0;
    queue *startOrder = malloc(sizeof(queue)); //the order in which tasks are intially started
    while(current!=null){
        int runTime = current->nodeJob->length;
        if(current->nodeJob->runT == 0){
            //this would be the first time that the task has been scheduled
            current->nodeJob->startT = time;
            current->nodeJob->runT++;
            job *temp = malloc(sizeof(job));
            temp->id =  current->nodeJob->id;
            addToQueue(startOrder, createNode(temp));
        }
        else{
                //the task has already been scheduled before, taking note of it
                current->nodeJob->runT++;
        }
        if(interval < runTime){
            current->nodeJob->length -= interval;
            runTime = interval;
            time += runTime;
            printf("Job %d ran for: %d\n",current->nodeJob->id, runTime);
            addToQueue(tasks, current);
        }
        else{
            current->nodeJob->waitT = time - (interval * (current->nodeJob->runT-1)); //takes into account that last run being counted in times run
            time+=runTime;
            current->nodeJob->endT = time;
            addToQueue(fTasks, current);
            printf("Job %d ran for: %d\n",current->nodeJob->id, runTime);
        }
        current = removeFirst(tasks);
    }
    printf("End of execution with RR.\n");

    printf("Begin analyzing RR:\n");
    double avRes = 0; //average response time
    double avTurn = 0;//average turnaround time
    double avWait = 0;//average wait time
    int numT = 0; //the actual number of tasks that were completed
    //sorting nodes by start time
    node *temp = removeFirst(startOrder);
    while(temp != null){
        node *temp2 = removeFirst(fTasks);
        while(temp2->nodeJob->id != temp->nodeJob->id){
            //makes sure to print out tasks in order in which they were first received
            addToQueue(fTasks, temp2);
            temp2 = removeFirst(fTasks);
        }
        //prints out details of job and calculates average
        job  *j = temp2 ->nodeJob;
        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", j->id, j->startT, j->endT, j->waitT);
        avRes += j->startT;
        avTurn += j->endT;
        avWait += j->waitT;
        numT++;
        free(j);
        free(temp);
        temp = removeFirst(startOrder);
    }
    avRes/=numT;
    avTurn/=numT;
    avWait/=numT;
    printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n",avRes, avTurn, avWait);
    printf("End analyzing RR.\n");



    return 0;
}

int firstInFirstOut(queue* tasks, int sorted){
    node *current = removeFirst(tasks);
    int time = 0;
    queue *fTasks = malloc(sizeof(queue)); //Queue of finished tasks
    fTasks->size = 0;
    printf("Execution trace with %s:\n", (sorted == 1 ? "SJF":"FIFO"));
    while(current!=null){
        job *j = current->nodeJob;
        j->startT = time;
        j->waitT = time;
        j->runT++;
        int runTime = current->nodeJob->length;
        time += runTime;
        j->endT = time;
        printf("Job %d ran for: %d\n",current->nodeJob->id, runTime);
        addToQueue(fTasks, current);
        current = removeFirst(tasks);
    }
    printf("End of execution with %s.\n", (sorted == 1 ? "SJF":"FIFO"));

    printf("Begin analyzing %s:\n", (sorted == 1 ? "SJF":"FIFO"));
    double avRes = 0; //average response time
    double avTurn = 0;//average turnaround time
    double avWait = 0;//average wait time
    int numT = 0; //the actual number of tasks that were completed
    node *temp = removeFirst(fTasks);
    while(temp!=null){
        job  *j = temp ->nodeJob;
        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", j->id, j->startT, j->endT, j->waitT);
        avRes += j->startT;
        avTurn += j->endT;
        avWait += j->waitT;
        numT++;
        free(j);
        free(temp);
        temp = removeFirst(fTasks);
    }
    avRes/=numT;
    avTurn/=numT;
    avWait/=numT;
    printf("Average -- Response: %.2f  Turnaround: %.2f  Wait: %.2f\n",avRes, avTurn, avWait);
    printf("End analyzing %s.\n", (sorted == 1 ? "SJF":"FIFO"));
    return 0;

    
}






/*
This function is purely for testing to make sure
that the adding and removing functions in the queue.c file
work as intended.

This function should not be called unless for that explicit purpose
*/
void testQueue(){
    srand(1);
    printf("TESTING QUEUE FUNCTIONS: ADDING\n");
    queue *q1;
    queue *q2;
    for(int i = 0; i < 10; i++){
        int temp = rand()%20;
        job *j1 = createJob(i, temp);
        job *j2 = createJob(i, temp);
        node *n1 = createNode(j1);
        node *n2 = createNode(j2);
        if(i == 0){
            q1 = createQueue(n1);
            q2 = createQueue(n2);
        }
        else{
            addToQueue(q1,n1);
            addToOrderedQueue(q2, n2);
        }
    }
    printf("Unordered Queue:\n");
    printQueue(q1);
    printf("Ordered Queue:\n");
    printQueue(q2);
    printf("TESTING QUEUE FUNCTIONS: REMOVING\n");
    printf("Unordered Queue:\n");
    for(int i = 0; i < 10; i++){
        node *temp = removeFirst(q1);
        if(temp!=null){
            printf("(%d, %d) ", temp->nodeJob->id, temp->nodeJob->length);
        }
        else{
            printf("ERROR!!\n");
        }
    }
    printf("\n");
    printf("Ordered Queue:\n");
    for(int i = 0; i < 10; i++){
        node *temp = removeFirst(q2);
        if(temp != null){
            printf("(%d, %d) ", temp->nodeJob->id, temp->nodeJob->length);
        }
        else{
            printf("ERROR!!\n");
        }
    }
    printf("\n");
}