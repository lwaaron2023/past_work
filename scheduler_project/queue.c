#include "queue.h"

/*
creates and returns a job given the provided attributes
*/
job *createJob(int id, int length){
    job *j = malloc(sizeof(job));
    j->id = id;
    j->length = length;
    j->startT = -1;
    j->endT = -1;
    j->runT = 0;
    j->waitT = 0;
    return j;
}
//creates a node holding a job
node *createNode(job *j){
    node *n = malloc(sizeof(node));
    n->nodeJob = j;
    n->next = null;
    return n;
}
/*
creates a new queue of size 1 with the first node
*/
queue *createQueue(node *n){
    queue *q = malloc(sizeof(queue));
    q->first = n;
    q->last = n;
    q->size = 1;
    return q;
}
/*
adds a node to the end of a queue
*/
void addToQueue(queue*q, node*n){
    if(q->size > 0){
        q->last->next = n;
        q->last = n;
        q->size++;
    }
    else if(q->size == 0){
        q->first = n;
        q->last = n;
        q->size++;
    }   
    else{
        printf("ERROR IN ADDING NODE, QUEUE SIZE IS LESS THAN 0");
    }
}
/*
adds a node into a queue into its proper spot such that the length of the 
job is less than the next node and greater than the previous node

NOTE: if the job length of the node is equal to that of another node already in 
the queue then it will place the new node AFTER the already placed node
*/
void addToOrderedQueue(queue*q, node*n){
    if(q->size > 0){
        node* temp = q->first;
        if(temp->nodeJob->length > n->nodeJob->length){
            n->next = temp;
            q->first = n;
            q->size++;
        }
        else{
            while(temp->next != null && temp->next->nodeJob->length <= n->nodeJob->length){
                temp = temp->next;
            }
            if(temp == null){
                q->last->next = n;
                q->last = n;
                q->size++;
            }
            else{
                n->next = temp->next;
                temp->next = n;
                q->size++;
            }
        }
    }
    else if(q->size == 0){
        q->first = n;
        q->last = n;
        q->size++;
    }   
    else{
        printf("ERROR IN ADDING NODE, QUEUE SIZE IS LESS THAN 0");
    }
}

/*
removes the first node from the queue and returns it
*/
node *removeFirst(queue*q){
    node *temp = null;
    if(q->size > 0){
        if(q->size == 1){
            q->size--;
            temp = q->first;
            q->first = null;
            q->last = null;
        }
        else{
            q->size--;
            temp = q->first;
            q->first = q->first->next;
        }
    }
    else if(q->size == 0){}
    else{
        printf("ERROR IN REMOVING NODE, QUEUE SIZE IS LESS THAN 0");
    }
    return temp;
}
/*
prints the contents of a queue
*/
void printQueue(queue*q){
    node* temp = q->first;
    printf("Queue of size %d: ", q->size);
    int index = 0;
    //to prevent an infinite loop in case of an error in modification of queue
    while(temp != null && index < q->size){
        printf("(id: %d, len: %d) ", temp->nodeJob->id, temp->nodeJob->length);
        temp = temp->next;
        index++;
    }
    printf("\n");
}