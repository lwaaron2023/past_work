// Starting code version 1.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include "mmu.h"
#include "pagetable.h"
#include "memsim.h"
#define DISK_SWAP_FILE_PATH ((const char*) "./disk.txt")
#define pfn (PFN & 0xf)
#define vBit ((valid & 1) << 5)
#define pBit ((protection & 1) << 6)
#define PMEMSIZE 64
#define EXTRA_DEBUG 0 //when set to any non zero, more information is printed out
#define MAX_SEC_FRAME_NUM (NUM_PROCESSES)*(NUM_FRAMES+1)
int pertains[NUM_FRAMES]; //to which pid each frame belongs to
int times = 0; //used for debugging
FILE* swapFile;


int frameToEvict = 0; //if using round robin eviction, this can be used to keep track of the next page to evict
// Page table root pointer register values (one stored per process, would be "swapped in" to MMU with process, when scheduled)
typedef struct{
	int ptStartF; //frame of page table
	int present; //present in physical memory
	int presentS; //present in secondary memory
	char active; //used to store which PTE have been initialized
} ptRegister;

// Page table root pointer register values 
// One stored for each process, swapped in to MMU when process is scheduled to run)
ptRegister ptRegVals[NUM_PROCESSES]; 
/*
 * Public Interface:
 */

/*
 * Sets the Page Table Entry (PTE) for the given process and VPN.
 * The PTE contains the PFN, valid bit, protection bit, present bit, and referenced bit.
 */
int PT_SetPTE(int pid, int VPN, int PFN, int valid, int protection) {
	//Referenced bit is whether it has been referenced in last cycle
	//Protection bit is level of protection
	//Valid is whether the page exists
	//Each PTE IS 2 bytes, stored as short
	//PTE: [page frame number [5 bit] | valid | protection]
	//Since the main memory has addresses 0-3, any address >3 with a valid bit is in secondary memory
	unsigned char e1=0;
	unsigned char* physmem = Memsim_GetPhysMem();
	int rtr = 0;
	if(EXTRA_DEBUG) printf("PFN is %d for VPN %d for PID %d\n", PFN, VPN, pid);
	//printf("PFN %d\n",PFN);
	assert(PT_PageTableExists(pid)); // page table should exist if this is being called
	if(ptRegVals[pid].ptStartF != PFN && PFN < MAX_SEC_FRAME_NUM){
		e1 |= (pfn &0x1f) | vBit | pBit;
	}
	if(VPN > 3){
		//printf("ILLEGAL VPN: MAX VPN ALLOWED IS 3\n");
		rtr = -1;
	}
	else{
		//printf("Setting PTE: %X\n", e1);
		ptRegVals[pid].active|=(1 << VPN);
		int tableAddr = ptRegVals[pid].ptStartF*PAGE_SIZE;
		physmem[tableAddr+VPN] = e1;
		pertains[PFN] = pid;
		rtr = 1;
	}
	return rtr;
	if(EXTRA_DEBUG) printf("PAGE TABLES AFTER UPDATES\n");
	if(EXTRA_DEBUG) printMPT();
}
/*
 * Updates the Page Table Entry (PTE) for the given process and VPN.
 * The PTE contains the PFN, valid bit, protection bit, present bit, and referenced bit.
 */
void PT_UpdatePTE(int pid, int VPN, int PFN){
	//Referenced bit is whether it has been referenced in last cycle
	//Present bit is whether it is in main memory
	//Protection bit is level of protection
	//Valid is whether it is in secondary memory
	//Each PTE IS 2 bytes, stored as short
	//PTE: [page frame number [4 bit] | valid | protection]
	if(EXTRA_DEBUG) printf("NEW PFN is %d for VPN %d for PID %d\n", PFN, VPN, pid);
	int base = PT_GetRootPtrRegVal(pid);
	unsigned char* physmem = Memsim_GetPhysMem();
	int rtr = 0;
	//printf("PFN %d\n",PFN);
	assert(PT_PageTableExists(pid)); // page table should exist if this is being called
	if(ptRegVals[pid].ptStartF != PFN && PFN < MAX_SEC_FRAME_NUM){
		physmem[base+VPN] = ((physmem[base+VPN]&0xe0)|(PFN&0x1f));
	}
	if(EXTRA_DEBUG) printf("PAGE TABLES AFTER UPDATES\n");
	if(EXTRA_DEBUG) printMPT();
}


/* 
 * Initializes the page table for the process, and sets the starting physical address for the page table.
 * 
 * After initialization, get the next free page in physical memory.
 * If there are no free pages, evict a page to get a newly freed physical address.
 * Finally, return the physical address of the next free page.
 */
int PT_PageTableInit(int pid, int pa){
	unsigned char* physmem = Memsim_GetPhysMem();
	
	int rtr = -1;
	if(pa%PAGE_SIZE == 0){
		//make sure that address is at the start of a frame
		//memset(physmem+(pa/PAGE_SIZE), 0, PAGE_SIZE); 
		printf("PAGE TABLE BEING INITIALIZED IN FRAME %d\n", pa/PAGE_SIZE);
		ptRegVals[pid].ptStartF = pa/PAGE_SIZE;
		ptRegVals[pid].present = 1;
		ptRegVals[pid].presentS = 0;
		pertains[pa/PAGE_SIZE] = pid;
		if(EXTRA_DEBUG) printf("MASTER PAGE TABLE AFTER TABLE ADDED\n");
		if(EXTRA_DEBUG) printMPT();
		rtr = Memsim_FirstFreePFN();
		if(rtr < 0){
			int freeF = PT_Evict(pid);
			if(freeF >= 0){
				rtr = freeF;
			}
			else{
				printf("CANNOT INTIALIZE TABLE, NO PAGES TO EVICT\n");
				rtr = -1;
			}
		}
		pertains[rtr/PAGE_SIZE] = pid;
	}
	return rtr;
 }

/* 
 * Check the ptRegVars to see if there is a valid starting PA for the given PID's page table.
 * Returns true (non-zero) or false (zero).
 */
 int PT_PageTableExists(int pid){
 	return (ptRegVals[pid].present | ptRegVals[pid].presentS); 
 }

/* 
 * Returns the starting physical address of the page table for the given PID.
 * If the page table does not exist, returns -1.
 * If the page table is not in memory, first swaps it in to physical memory.
 * Finally, returns the starting physical address of the page table.
 */
int PT_GetRootPtrRegVal(int pid){
	// If the page table does not exist, return -1
	int rtr = -1;
	if(ptRegVals[pid].present){
		//CASE 1 IN MAIN MEMORY
		rtr = ptRegVals[pid].ptStartF*PAGE_SIZE;
	}
	else if(ptRegVals[pid].presentS){
		//CASE 2 IN SECONDARY MEMORY
		rtr = PT_Evict_And_Replace(pid, ptRegVals[pid].ptStartF);
		if(rtr >= 0){
			ptRegVals[pid].presentS = 0;
			ptRegVals[pid].present = 1;
			ptRegVals[pid].ptStartF = rtr/PAGE_SIZE;
		}
	}
	//CASE 3 DNE -> RETURN -1
	return rtr; 
}

/*
 * Evicts the next page. 
 * Updates the corresponding information in the page table, returns the PA of the evicted page.
 * 
 * The supplied input and output used in autotest.sh *RR tests, uses the round-robin algorithm. 
 * You may also implement the simple and powerful Least Recently Used (LRU) policy, 
 * or another fair algorithm.
 */
int PT_Evict(int pid) {
	int rtr = -1;
	//This puts page into secondary memory, does not put any information from secondary memory 
	//back into main memory, assumes that the location to put in secondary memory
	//has not been "claimed" by any pages
	int frameNum = Memsim_FirstFreeSPFN()+NUM_FRAMES; //frame in secondary storage
	if(EXTRA_DEBUG) printf("EVICTING FAME, NO REPLACE, TO FRAME %d IN SECONDARY MEMORY\n", frameNum);
	rtr = PT_Evict_And_Replace(pid, frameNum); //just call other function, the "replacement" will be all zeroes
	//There is no free frames
	return rtr; 
}

/*
 * Evicts the next page. 
 * Updates the corresponding information in the page table, replaces the evicted page with a page from secondary memory.
 * 
 * The supplied input and output used in autotest.sh *RR tests, uses the round-robin algorithm. 
 * You may also implement the simple and powerful Least Recently Used (LRU) policy, 
 * or another fair algorithm.
 */
int PT_Evict_And_Replace(int pid, int frameNum) {
	int rtr = -1;
	//This puts page into secondary memory, and puts a page from secondary memory 
	//back into main memory, assumes that the location to put in secondary memory
	//has been "claimed" by other pages
	
	//There is a free frame
	times = 0;
	findFrameToEvict(pid);
	printf("EVICTING FRAME %d BELONGS TO PROCESS %d\n", frameToEvict, pertains[frameToEvict]);
	int former = pertains[frameToEvict]; //former pid that "owned" the frame
	pertains[frameToEvict] = pid; //new "owner" of the frame
	//rtr = frameToEvict * PAGE_SIZE; //the physical address of the 
	if(former >= 0){
		if(ptRegVals[former].ptStartF == frameToEvict){
			//evicting a page table
			if(EXTRA_DEBUG) printf("FRAME TO BE EVICTED BELONGS TO A PAGE TABLE FOR PID %d\n", former);
			ptRegVals[former].ptStartF = frameNum; //mark new location
			ptRegVals[former].present = 0; //mark not in main memory
			ptRegVals[former].presentS = 1; //mark in secondary memory
		}
		else{
			//evicting a page
			if(EXTRA_DEBUG) printf("FRAME TO BE EVICTED BELONGS TO A PAGE FOR PID %d\n", former);
			int VPN = PT_PAtoVPN(former, frameToEvict);
			if(VPN >=0){
				printf("EVICT AND REPLACE: UPDATING PTE FOR PID %d and VPN %d to point to frame %d\n", former, VPN, frameNum);
				PT_UpdatePTE(former, VPN, frameNum); //updates location to new location
				if(EXTRA_DEBUG) printf("MPT AFTER UPDATE:\n");
				if(EXTRA_DEBUG) printMPT();
			}
		}
		rtr = frameToEvict * PAGE_SIZE; //the PA for the newly swapped in page
		swapInFrame(frameToEvict*PAGE_SIZE, frameNum-NUM_FRAMES);//swaps frame into secondary memory, and information in secondary memory into frame
		frameToEvict = (frameToEvict+1)%NUM_FRAMES;
	}
	if(EXTRA_DEBUG) printf("\n");
	//printf("RETURNING ADDRESS %d\n", rtr);
	return rtr;
}

/*
Searches through the page table to see if a VPN masks to a given frame 
*/

int PT_PAtoVPN(int pid, int frameNum){
	int tableStart = PT_GetRootPtrRegVal(pid); //get start address of table, may be loaded into memory
	int rtr = -1;
	if(EXTRA_DEBUG) printf("MPT during function call\n");
	if(EXTRA_DEBUG) printMPT();
	if(tableStart >= 0){
		if(EXTRA_DEBUG) printf("Searching for frame number %d\n", frameNum);
		unsigned char *physmem = Memsim_GetPhysMem();
		for(int i = 0; i < NUM_FRAMES; i++){
			if(EXTRA_DEBUG) printf("VPN %d: Mapped to FRAME %d with PTE:0x%x\n", i, physmem[tableStart+i]&0x1f, physmem[tableStart+i]);
			if(!((physmem[tableStart+i]&0x1f)^frameNum)){
				rtr = i;
				break;
			}
		}
		//if(EXTRA_DEBUG) printf("\n");
	}
	return rtr;
}

/*
 * Searches through the process's page table. If an entry is found containing the specified VPN, 
 * return the address of the start of the corresponding physical page frame in physical memory. 
 *
 * If the physical page is not present, first swaps in the phyical page from the physical disk,
 * and returns the physical address.
 * 
 * Otherwise, returns -1.
 */
int PT_VPNtoPA(int pid, int VPN){
	int rtr = -1;
	if(PT_PageTableExists(pid)){
		int startAddr = PT_GetRootPtrRegVal(pid); //finds the start of the page table
		unsigned char *physmem = Memsim_GetPhysMem(); //gets memory
		int frameNum = physmem[startAddr+VPN] & 0x1f; //gets frame number
		printf("PID %d VPN %d found at FRAME %d\n", pid, VPN, frameNum);
		if(frameNum > 3 && startAddr >=0){
			//frame in secondary
			if(EXTRA_DEBUG) printf("Moving frame to primary to get PA\n");
			rtr = PT_Evict_And_Replace(pid, frameNum);
			printf("VPN TO PA: UPDATING PTE FOR PID %d and VPN %d to point to frame %d\n", pid, VPN, frameNum);
			PT_UpdatePTE(pid, VPN, rtr/PAGE_SIZE);
			if(EXTRA_DEBUG) if(EXTRA_DEBUG) printf("FRAME MOVED TO PA %d\n", rtr);
			if(EXTRA_DEBUG) printf("MPT AFTER UPDATE:\n");
			if(EXTRA_DEBUG) printMPT();
		}
		else if(frameNum >=0 && startAddr >=0){
			//frame is in primary
			rtr = frameNum *PAGE_SIZE;
		}
		if(EXTRA_DEBUG) printf("\n");
	}
	//CASE 3 PAGE TABLE DNE -> RETURN -1
	return rtr;
}

/*
 * Finds the page table entry corresponding to the VPN, and checks
 * to see if the protection bit is set to 1 (readable and writable).
 * If it is 1, it returns TRUE, and FALSE if it is not found or is 0.
 */
int PT_PIDHasWritePerm(int pid, int VPN){
	int rtr = 0;
	int start = PT_GetRootPtrRegVal(pid);
	unsigned char* physmem = Memsim_GetPhysMem();
	printf("Checking permissions for page %d of pid %d with PTE: 0x%x\n",VPN, pid, physmem[start+VPN]);
	if(!((physmem[start+VPN]&0x40)^0x40)){
		rtr = 1;
	}
	//printf("return %d\n",rtr);
	if(EXTRA_DEBUG) printf("\n");
	return rtr; 
}

/* 
 * Initialize the register values for each page table location (per process).
 * For example, -1 for the starting physical address of the page table, and FALSE for present.
 */
void PT_Init() {
	frameToEvict = 0;
	for(int i = 0; i < NUM_PAGES; i++){
		ptRegVals[i].ptStartF = -1; //table does not exist
		ptRegVals[i].present = 0; //not present in main memory
		ptRegVals[i].presentS = 0; //not present in secondary memory
		ptRegVals[i].active = 0; //no PTE exist yet
	}
	for(int i = 0; i < NUM_FRAMES; i++){
		pertains[i] = -1;
	}
	//Clears out the allocated space on the disk 
	FILE* f = fopen(DISK_SWAP_FILE_PATH, "w");
	unsigned char *temp[SEC_STORE_SIZE];
	memset(temp, 0, SEC_STORE_SIZE);
	fwrite(temp, 1, SEC_STORE_SIZE, f);
	fclose(f);
	return; 
}




/*HELPER FUNCTIONS*/

/*
Confirms that the frame to evict is either a
page table whose entries are in secondary memory
or a page
Will not evict its own page table
*/

void findFrameToEvict(int pid){
	if(EXTRA_DEBUG) printf("CURRENT FRAME UNDER CONSIDERATION TO EVICT %d\n",frameToEvict);
	if(times < 5){
		if(pertains[frameToEvict] >= 0){
			
			if(ptRegVals[pertains[frameToEvict]].ptStartF == frameToEvict){
				//check if page table is empty
				if(EXTRA_DEBUG) printf("FRAME TO EVICT WAS PAGE TABLE, CHECKING FOR ANY PAGES IN MEMORY\n");
				unsigned char* physmem = Memsim_GetPhysMem();
				int valid = 1;
				if(ptRegVals[pid].ptStartF != frameToEvict){
					for(int i = 0; i < NUM_PAGES; i++){
						if(EXTRA_DEBUG) printf("CHECKING PAGE %d PTE 0x%x\n", i, physmem[(frameToEvict*PAGE_SIZE)+i]);
						if((physmem[(frameToEvict*PAGE_SIZE)+i]&0x1f) < NUM_FRAMES){
							valid = 0;
							break;
						}
					}
				}
				else{
					if(EXTRA_DEBUG) printf("PAGE TRYING TO EVICT ITS OWN PAGE TABLE, SKIPPING FRAME\n");
					valid = 0;
				}
				if(valid == 0){
					//advance one frame and check again
					frameToEvict = (frameToEvict+1)%NUM_FRAMES;
					times++;
					findFrameToEvict(pid);
				}
			}
			
		}
		if(EXTRA_DEBUG) printf("\n");
	}
	else{
		printf("INFINITE LOOP ON TRYING TO REPLACE A FRAME\n");
		exit(-1);
	}

}

/*
Checks active bitmap in order to see if PTE for VPN has 
been created or needs to be created
*/
int existsVPN(int pid, int VPN){
	if(EXTRA_DEBUG) printf("BITMAP FOR PID %d: 0x%x\n", pid, ptRegVals[pid].active);
	int rtr = -1;
	if(PT_PageTableExists(pid)){
		rtr = ((ptRegVals[pid].active >> VPN) & 1); //sees if bit VPN in active is set to 1
	}
	return rtr;
}

/*
Puts the frame number secF in secondary memory to 
the frame starting in main memory address addr
and the frame in main memory to that address in secondary memory
*/
void swapInFrame(int addr, int secF){
	//printf("ADDR: %d\n", addr);
	FILE* swapFile = MMU_GetSwapFileHandle();
	unsigned char* mem = Memsim_GetPhysMem();
	swapFile = fopen(DISK_SWAP_FILE_PATH, "r");
	unsigned char buffer[SEC_STORE_SIZE];
	//since the file will always contain 192 bytes (all 0 to start off)
	//this should work
	int read = fread(buffer, 1, SEC_STORE_SIZE, swapFile);
	if(read == 0){
		printf("ERROR: COULD NOT READ DISK!!!\n");
		exit(-1);
	}
	fclose(swapFile);
	for(int i = 0; i < PAGE_SIZE; i++){
		mem[addr+i]^=buffer[(secF*PAGE_SIZE)+i];
		buffer[(secF*PAGE_SIZE)+i]^=mem[addr+i];
		mem[addr+i]^=buffer[(secF*PAGE_SIZE)+i];
	}
	// for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buffer[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }
	// printf("\n");
	fopen(DISK_SWAP_FILE_PATH, "w");
	int write = fwrite(buffer, 1, SEC_STORE_SIZE, swapFile);
	if(write == 0){
		printf("ERROR: COULD NOT WRITE TO DISK!!!\n");
		exit(-1);
	}
	fclose(swapFile);
}
/*
Used for debugging 
*/
void printMPT(){
	unsigned char *mem = Memsim_GetPhysMem();
    unsigned char buff[SEC_STORE_SIZE];
	FILE* f = fopen(DISK_SWAP_FILE_PATH, "r");
    int r=fread(buff, 1, SEC_STORE_SIZE, f);
    fclose(f);
	for(int i = 0; i < NUM_PROCESSES; i++){
		printf("LOCATION OF TABLE %d (FRAME #): %d PRIMARY? %d SECONDARY? %d\n", i, ptRegVals[i].ptStartF, ptRegVals[i].present, ptRegVals[i].presentS);
		if(ptRegVals[i].present){
			int base = ptRegVals[i].ptStartF*PAGE_SIZE;
			printf("STARTING FOR ADDRESS %d IN MAIN MEMORY\n", base);
			for(int j = 0; j < NUM_PAGES; j++){
				printf("%d: %d %d\n", j, ((mem[base+j])>>5)&0x7, mem[base+j]&0x1f);
			}
		}
		else if(ptRegVals[i].presentS){
			int base = (ptRegVals[i].ptStartF-NUM_FRAMES)*PAGE_SIZE;
			printf("STARTING FOR ADDRESS %d IN SECONDARY MEMORY\n", base);
			for(int j = 0; j < NUM_PAGES; j++){
				printf("%d: %d %d\n", j, (buff[base+j]>>5)&0x7, buff[base+j]&0x1f);
			}
		}
	}
}
/*
Used for debugging 
*/
void printMainMem(){
	unsigned char *mem = Memsim_GetPhysMem();
	for(int i = 0; i < PMEMSIZE; i++){
		printf("0x%x ", mem[i]);
		if((i+1)%16 == 0){
			printf("\n");
		}
	}
}
/*
Used for debugging 
*/
void printSecondMem(){
    unsigned char buff[SEC_STORE_SIZE];
	FILE* f = fopen(DISK_SWAP_FILE_PATH, "r");
    int r=fread(buff, 1, SEC_STORE_SIZE, f);
    for(int i = 0; i < SEC_STORE_SIZE; i++){
		printf("0x%x ", buff[i]);
		if((i+1)%16 == 0){
			printf("\n");
		}
	}
}
