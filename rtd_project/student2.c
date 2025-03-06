#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project2.h"
#include <unistd.h>

/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

#define MAX_FAILS 10; //The number of consequetive packet fails tolerated

typedef struct msg msg;
typedef struct pkt pkt;

struct QueueNode{
  struct QueueNode *next;
  msg message;
};

struct Queue{
  struct QueueNode *first;
};

typedef struct QueueNode node;
typedef struct Queue queue;

int isEmpty(queue* q);
int calculateChecksum(pkt p);
int checkPacket(pkt p, int state, int acknum);
void addNode(queue* q, node *n);
node* createNode(msg messahe);
msg removeFirst(queue* q);
void sendNextNode();
void resendCurrentPacket();
pkt createACK(int s, int state);

int channelFree; //Represents if A has sent message and not received response, 1 means it can send a message
int waitingForReply; //Represents whether the sending side is waiting for a ACK reply
int aState; //The current sequence number that A is working with
int bState; //The current sequence number that B is working with
queue *aSendQueue; //The queue of messages to be sent
pkt currentPacket; //The current packet being sent through the network
pkt lastACK; //The last packet to be ACKed on B side
/*
Creates a node to be used in a queue by allocating space
in the heap for the size of the node structure

It then initializes the pointer for the next node to be null
and the value of the message to be the message
*/
node* createNode(msg message){
  node *n = (node*) malloc(sizeof(node));
  n->message = message;
  n->next= NULL;
  return n;
}
/*
Adds a node to the end of the queue
*/
void addNode(queue* q, node *n){
  if(q->first == NULL){
    q->first = n;
  }
  else{
    //printf("STARTING DO WHILE\n");
    node* temp = q->first;
    while(temp->next!=NULL){
      temp = temp -> next;
    }
    //printf("TEMP IS NULL? %s", (temp == NULL ? "YES":"NO"));
    //exit(1);
    temp->next = n;
  }
}
/*
Returns 0 if the queue points to NULL (is empty)
Otherwise returns 1
*/
int isEmpty(queue *q){
  return (q->first == NULL ? 0:1);
}

/*
Removes the first node from the queue and returns its message
*/
msg removeFirst(queue* q){
  node *temp = NULL;
  msg m;
  if(q-> first != NULL){
    temp = q->first;
    m = temp->message;
    q->first = temp->next;
    free(temp);
  }
  return m;
}


/*
Calculates a chechsum by multiplying the ASCII value of a character by its index position (plus 1)
Always makes checksum equal checksum modulus of checksum and max int to make sure no integer overflow occurs
Then it adds the 2 times the acknum and 5 times the seqnum to the checksum
*/
int calculateChecksum(pkt p){
  int checksum = 0;
  //printf("starting checksum: %d, ack: %d, seqnum: %d\n", checksum, ack, seqnum);
  for(int i = 0; i < MESSAGE_LENGTH; i++){
    //printf("%c\t", p.payload[i]);
    checksum+=(int)(p.payload[i]*(i+1));
  }
  //printf("final checksum: %d\n", checksum);
  checksum+= 21*p.acknum;
  checksum+= 22*p.seqnum;
  return checksum;
}

/*
Checks to make sure that the packet is of the requested seqnum and has the correct data
*/
int checkPacket(pkt p, int state, int acknum){
  int response; //whether or not the packet is correct
  if(p.checksum == calculateChecksum(p)){
    if(p.seqnum == state){
      if(p.acknum == acknum){
        response = 1;
      }
      else{
        response = -1;
      }
    }
    else{
      response = -2;
    }
  }
  else{
    response = -3;
  }
  return response;
}



queue* AmessageQueue;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
  //printf("A output called\n");
  //printf("\033[31;1;4mAdding new message %s\033[0m\n", message.data);
  addNode(aSendQueue, createNode(message));
  if(channelFree == 1){
    sendNextNode();
  }
}

/*
Sends the first node in the send queue to the B side
*/
void sendNextNode(){
  
  channelFree = 0;
  msg m = removeFirst(aSendQueue);
  currentPacket.acknum = 0;
  memccpy(currentPacket.payload, m.data, 1, MESSAGE_LENGTH);
  currentPacket.seqnum = aState;
  currentPacket.checksum = calculateChecksum(currentPacket);
  //printf("\033[33;1;4mSending packet with message %s at time %f with seqnum %d with checksum %d and String length %d\033[0m\n", currentPacket.payload, getClockTime(), currentPacket.seqnum, currentPacket.checksum, strlen(currentPacket.payload));
  startTimer(AEntity, 200); //Chose 200 because it usually takes 100 for each way (on average)
  tolayer3(AEntity, currentPacket);
  
}
/*
Sends the packet in the current packet field from A to B
Basically this function servers to differentiate between having to resend a package vs having to send a new one
*/
void resendCurrentPacket(){
  //printf("\033[34;1;4mResending packet with message %s at time %f\033[0m\033[0m\n", currentPacket.payload, getClockTime());
  startTimer(AEntity, 200); //Chose 200 because it usually takes 100 for each way (on average)
  channelFree = 0;
  tolayer3(AEntity, currentPacket);
  
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
  
  //printf("\033[30;1;4mReceived response packet with message %s ack %d at time %f with seqnum %d\033[0m\n", packet.payload, packet.acknum, getClockTime(), packet.seqnum);
  //printf("LOOKING FOR SEQNUM %d\n", aState);
  int r = checkPacket(packet, aState, 1);
  stopTimer(AEntity);
  if(r == 1){
    aState = (aState == 1 ? 0:1);
    if(isEmpty(aSendQueue) == 1){
      sendNextNode();
    }
    else{
      channelFree = 1;
    }
  }
  else{
    resendCurrentPacket();
  }
  
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
  //printf("A TIMEOUT\n");
  //printf("\033[36;1;4mTimeout at time %f\033[0m\n", getClockTime());
  resendCurrentPacket();
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
  channelFree = 1;
  aState = 0;
  aSendQueue = malloc(sizeof(queue));
  waitingForReply = 0;
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {

}
/*
Creates one of three packets:
Packet 1: ACK = 1, and payload is all 1's is equivalent to ACK with correct sequence number to change sender's state
Packet 2: ACK = 1, and payload is DUP (only issued with sender fails to receive ack messages)
Packet 3: ACK = 0, and payload is all 0's is equivalent to ACK with wrong sequence number to chnage sender's state

The reason why packet 2 exists is that sometimes the ACK message goes missing, which would cause the current implementation to repeadily try to resend the package (even though it is no longer needed)
*/
pkt createACK(int s, int state){
  pkt p;
  if(s == 1){
    p.acknum = 1;
    strcpy(p.payload, "1111111111111111111");
  }
  else if(s == 2){
    p.acknum = 1;
    strcpy(p.payload, "DUP");
  }
  else{
    p.acknum = 0;
    strcpy(p.payload, "0000000000000000000");
  }
  p.seqnum = state;
  p.checksum = calculateChecksum(p);
  return p;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
  //printf("RECIEVED PACKET ON OTHER SIDE\n");
  
  int correct = checkPacket(packet, bState, 0);
  //printf("\033[34;1;4mReceived packet with message %s at time %f\033[0m\n", packet.payload, getClockTime());
  pkt p;
  if(correct == 1){ 
    msg m;
    lastACK = packet;
    memccpy(m.data, packet.payload, 1, MESSAGE_LENGTH);
    //printf("\033[35;1;4mPacket Cleared Inspection with checksum %d and seqnum %d\033[0m\n", packet.checksum, packet.seqnum);
    tolayer5(BEntity, m);
    p = createACK(1, bState);
    bState = (bState == 1 ? 0:1);
  }
  //This is a very importatn statement, check createACK for more details
  else if(strcmp(packet.payload, lastACK.payload)&&packet.checksum == lastACK.checksum){
    p = createACK(2, packet.seqnum); 
  }  
  else{
   // printf("PACKET FAILED BECAUSE %s\n", (correct < -2?"INVALID CHECKSUM":(correct < -1? "WRONG STATE":"WRONG ACK")));
    p = createACK(0, bState);
  }
  tolayer3(BEntity, p);


}


/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
  printf("B TIMEOUT\n");
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
  bState = 0;
}

