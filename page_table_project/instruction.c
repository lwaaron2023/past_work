// Starting code version 1.0

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "memsim.h"
#include "pagetable.h"
#include "mmu.h"
#define vpn va/PAGE_SIZE
#define RESET_COLOR printf("\33[0m");
#define SET_BLUE  printf("\33[0;34m");
#define SET_GREEN  printf("\33[0;32m");
#define SET_YELLOW  printf("\33[0;33m");
#define EXTRA_DEBUG 0 //when set to any non zero, more information is printed out
/*
 * Searches the memory for a free page, and assigns it to the process's virtual address. If value is
 * 0, the page has read only permissions, and if it is 1, it has read/write permissions.
 * If the process does not already have a page table, one is assigned to it. If there are no more empty
 * pages, the mapping fails.
 */
int Instruction_Map(int pid, int va, int value_in){
	//Values to set: int pid, int VPN, int PFN, int valid, int protection, int present, int referenced
	SET_BLUE
	printf("\nSTARTING INSTRUCTION MAP WITH PARAMETERS %d %d %d\n\n", pid, va, value_in);
	if(va < VIRTUAL_SIZE && pid < NUM_PROCESSES && va >= 0 && pid >= 0){
		if(value_in == 1 | value_in == 0){
			//valid value for value in
			if(PT_PageTableExists(pid)){
				//PAGE TABLE EXISTS
				if(!existsVPN(pid, vpn)){
					int addr = Memsim_FirstFreePFN();
					if(addr < 0){
						addr = PT_Evict(pid);
					}
					if(addr >= 0){
						PT_SetPTE(pid, vpn, addr/PAGE_SIZE, 1, value_in);
						printf("MAPPED PID %d VPN %d to PA %d", pid, vpn, addr);
					}
					else{
						printf("ERROR: RAN OUT OF MEMORY, COULD NOT ALLOCATE PAGE, EVICT RETURNED %d\n", addr);
					}
				}
				else{
					printf("VPN %d for PID %d already exists\n", vpn, pid);
				}
			}
			//NEED TO DEBUG THIS SECTION OF CODE!!!!!!!!!!!
			else{
				//PAGE TABLE DOES NOT EXIST
				int tAddr = Memsim_FirstFreePFN();
				int addr = -1;
				//NOTE A NEED FOR TWO IDENTICAL IF STATEMENTS
				//SINCE THE ADDR CAN BE NEGATIVE AFTER A FAILED EVICT
				if(tAddr < 0){
					tAddr = PT_Evict(pid);
				}
				if(tAddr >= 0){
					addr = PT_PageTableInit(pid, tAddr);
					
				}
				else{
					printf("ERROR: RAN OUT OF MEMORY, COULD NOT ALLOCATE PAGE TABLE\n");
				}
				if(addr >= 0){
					//valid address to put page in
					PT_SetPTE(pid, vpn, addr/PAGE_SIZE, 1, value_in);
					printf("MAPPED PID %d VPN %d to PA %d", pid, vpn, addr);
				}
				else{
					printf("ERROR: RAN OUT OF MEMORY, COULD NOT ALLOCATE PAGE, EVICT RETURNED %d\n", addr);
				}
			}
		}
		else{
			printf("Invalid value for value_in: %d\n", value_in);
		}
		printf("\n\n");
		RESET_COLOR
		if(EXTRA_DEBUG){
			printf("MEMORY AFTER INSTRUCTION:\n");
			printMPT();
			printf("\n");
			printMainMem();
			printf("\n");
			printSecondMem();
			printf("\n\n");
		}
	}
	else{
		//When illegal values are inputted for va or pid
		RESET_COLOR
		printf("ILLEGAL PID OR VA INPUTTED\n\n");
	}
	return 0;
}

/**
* If the virtual address is valid and has write permissions for the process, store
* value in the virtual address specified.
*/
int Instruction_Store(int pid, int va, int value_in){
	SET_YELLOW
	printf("\nSTARTING INSTRUCTION STORE WITH PARAMETERS %d %d %u\n\n", pid, va, value_in);
	if(va < VIRTUAL_SIZE && pid < NUM_PROCESSES && va >= 0 && pid >= 0){
		if(existsVPN(pid, vpn)){
			if(PT_PIDHasWritePerm(pid,vpn)){
				unsigned char* physmem = Memsim_GetPhysMem(); 
				int addr = PT_VPNtoPA(pid, vpn); //get the physical address of the VPN
				if(addr >= 0){
					if(value_in <= 255 && value_in >= 0){
						unsigned char c = value_in &0xff;
						physmem[addr+(va%PAGE_SIZE)] = c;
						printf("Stored %d at VPN %d for PID %d at PA %d\n", c, vpn, pid, addr+(va%PAGE_SIZE));
					}
					else{
						printf("Invalid input %d, must be 0-255\n", value_in);
					}
				}
				else{
					printf("FAILED TO FIND VPN %d\n", vpn);
				}

			}
			else{
				printf("Error: PID %d VPN %d does not have write permissions\n", pid, vpn);
			}
		}
		printf("\n\n");
		RESET_COLOR
		if(EXTRA_DEBUG){
			printf("MEMORY AFTER INSTRUCTION:\n");
			printMPT();
			printf("\n");
			printMainMem();
			printf("\n");
			printSecondMem();
			printf("\n\n");
		}
	}
	else{
		//When illegal values are inputted for va or pid
		RESET_COLOR
		printf("ILLEGAL PID OR VA INPUTTED\n\n");
	}
	return 0;
}

/*
 * Translate the virtual address into its physical address for
 * the process. If the virutal memory is mapped to valid physical memory, 
 * return the value at the physical address. Permission checking is not needed,
 * since we assume all processes have (at least) read permissions on pages.
 */
int Instruction_Load(int pid, int va){
	SET_GREEN
	printf("\nSTARTING INSTRUCTION LOAD WITH PARAMETERS %d %d\n\n", pid, va);
	unsigned char* physmem = Memsim_GetPhysMem(); 
	if(va < VIRTUAL_SIZE && pid < NUM_PROCESSES && va >= 0 && pid >= 0){
		if(existsVPN(pid, vpn)){
			int addr = PT_VPNtoPA(pid, vpn); //get the physical address of the VPN
			if(addr >= 0){
				printf("Value for PID %d VA %d is at PA %d and is %u", pid, va, addr+(va%PAGE_SIZE), physmem[addr+(va%PAGE_SIZE)]);
			}
			else{
				printf("FAILED TO FIND VPN %d\n", vpn);
			}
		}
		//check for a valid value (instructions validate the value_in)
		//todo 
		// Hint use MMU_TranslateAddress to do a successful VA -> PA translation.
		printf("\n\n");
		RESET_COLOR
		if(EXTRA_DEBUG){
			printf("MEMORY AFTER INSTRUCTION:\n");
			printMPT();
			printf("\n");
			printMainMem();
			printf("\n");
			printSecondMem();
			printf("\n\n");
		}
	}
	else{
		//When illegal values are inputted for va or pid
		RESET_COLOR
		printf("ILLEGAL PID OR VA INPUTTED\n\n");
	}
	return 0;
}