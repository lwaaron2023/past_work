#include <stdio.h>
#include <stdlib.h>

#define null NULL
#define BUFFER_SIZE 256
#define MAX_TASK_DIGITS 10

struct job{
    int id; //the id of the job
    int length; //how long it takes for the job to run
    int startT; //the start time
    int endT; //the end time
    int runT; //the number of times task has been run
    int waitT; //the wait time
}; 
typedef struct job job;
struct node{
    job *nodeJob; //the job part of the node
    struct node *next; //the next node in the sequence
}; 
typedef struct node node;
struct queue{
    node *first; //the first node in the queue
    node *last; //the last node in the queue
    int size; //the size of the queue
};
typedef struct queue queue;


job *createJob(int id, int length);
node *createNode(job *j);
queue *createQueue(node *n);
void addToQueue(queue*q, node*n);
void addToOrderedQueue(queue*q, node*n);
node *removeFirst(queue*q);
void printQueue(queue *q);
