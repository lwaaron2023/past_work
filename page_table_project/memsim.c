// Starting code version 1.0

#include <assert.h>
#include <string.h>

#include "memsim.h"
#define MAX_SEC_FRAME_NUM NUM_PAGES*(NUM_FRAMES+1)
/* Private Internals: */

// Currently 'boolean' array (bitmap would be better for scale and perf)
short freePages[NUM_PAGES];
// Free frames in secondary storage
short freePagesS[MAX_SEC_FRAME_NUM];
// The simulated physical memory array (in bytes), aka physical R.A.M.
unsigned char physmem[PHYSICAL_SIZE];

/*
 * Performs sanity checks based on the simulations defined constants.
 * If these checks fail, the simulation is not valid and should not proceed.
 * Asserts are used to halt the program immediately if these checks fail.
 */
void MemsimConfigSanityChecks() { // you should not modify this function
	assert(PHYSICAL_SIZE % PAGE_SIZE == 0); // physical size in bytes must be a multiple of page size bytes
	assert(VIRTUAL_SIZE == PHYSICAL_SIZE); // standard implementation gives whole PA space to each process (virtually)
    assert(MAX_VA == MAX_PA);
    assert(NUM_PAGES == NUM_FRAMES);
}

/*
 *  Public Interface: 
 */

/* 
 * Zeroes out the simulated physical memory array.
 */
void Memsim_Init() { // zero free pages list 
    MemsimConfigSanityChecks();
    memset(physmem, 0, sizeof(physmem)); // zero out physical memory
	memset(freePages, 0, sizeof(freePages)); // zero implies free / FALSE / not used
    memset(freePagesS, 0, sizeof(freePagesS)); // zero implies free / FALSE / not used
}

 /* Gets current shared reference to start of simulated physical memory. */
unsigned char* Memsim_GetPhysMem() {
    return physmem;
}

/*
 * Searches through the free page list to find the first free page in memory. 
 * It claims the page, marking it, and returns the physical address of the beginning 
 * of the page. If there are no free pages, returns -1;
 */
int Memsim_FirstFreePFN(){
    for(int i = 0; i < NUM_PAGES; i++){
        if (freePages[i] == 0){
            freePages[i] = 1;
            return i * PAGE_SIZE;
        }
    }
	return -1;
}

/*
 * Searches through the free page list to find the first free page in memory. 
 * It claims the page, marking it, and returns the physical address of the beginning 
 * of the page. If there are no free pages, returns -1;
 */
int Memsim_FirstFreeSPFN(){
    for(int i = 0; i < MAX_SEC_FRAME_NUM ; i++){
        if (freePagesS[i] == 0){
            freePagesS[i] = 1;
            return i;
        }
    }
	return -1;
}

/*
 * Marks that a frame has been opened
 * This is the result of the init page table evicting the next frame if 
 * it is occupied
 */
int Memsim_MarkOpen(int page){
    if(page < NUM_PAGES){
        freePages[page] = 0;
    }
}
