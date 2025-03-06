// Starting code version 1.0

#ifndef MEMSIM_H
#define MEMSIM_H

#include <assert.h>

/*
 * Public Interface:
 */

#define PAGE_SIZE 16

#define PHYSICAL_SIZE 64
#define VIRTUAL_SIZE 64 

#define SEC_STORE_SIZE 256 //size of secondary storage
//addresses 0-63 are used to reference primary storage
//addressses 64-255 are used to reference secondary storage, address 64 is address 0 of secondary storage
//need 8 bits to store the address
#define MAX_VA 255
#define MAX_PA 255

#define NUM_PAGES (PHYSICAL_SIZE/PAGE_SIZE)
#define NUM_FRAMES NUM_PAGES

// Address getters
#define PAGE_START(i) (i * PAGE_SIZE)
#define PAGE_OFFSET(addr) (addr % PAGE_SIZE)
#define VPN(addr) (addr / PAGE_SIZE)
#define PFN(addr) (addr / PAGE_SIZE)
#define PAGE_NUM(addr) (addr / PAGE_SIZE)

// Public functions 
void Memsim_Init();
unsigned char* Memsim_GetPhysMem();
int Memsim_FirstFreePFN();
int Memsim_FirstFreeSPFN();
int Memsim_MarkOpen(int page);
#endif // MEMSIM_H