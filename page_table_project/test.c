#include <stdio.h>
#include "mmu.h"
#include "memsim.h"
#include "pagetable.h"
#include "instruction.h"
#define DISK_SWAP_FILE_PATH ((const char*) "./disk.txt")

/*
NOTE TO GRADER:

THIS FILE WAS USED FOR TESTING EARLY ON

THE TESTS SHOULD NOT WORK ANYMORE AS WHAT THEY WERE TESTING WAS OVERHAULED

DO NOT COMPILE THIS FILE BECAUSE IT IS USELESS
*/

int readFile(int size, char buffer[size]);
void testSwapIn();
void testSwapOut();
void testBasicPageTable();
void testBasicEviction();
void testMoreComplexEviction();
void testInstructionMap();
void testInstructionStore();


int main(int argc, char *argv[]){
    MMUInit(); //reset for next test
    testSwapIn(); 
    printf("\n");
    MMUInit(); //reset for next test
    testSwapOut();
    printf("\n");
    MMUInit(); //reset for next test
    testBasicPageTable();
    printf("\n");
    MMUInit(); //reset for next test
    testBasicEviction();
    printf("\n");
    MMUInit(); //reset for next test
    testMoreComplexEviction();
    printf("\n");
    MMUInit(); //reset for next test
    testInstructionMap();
    printf("\n");
    MMUInit(); //reset for next test
    testInstructionStore();
    printf("\n");
    return 0;
    //NEED TO UNCOMMENT PARTS OF MMU
}

/*
buff points to the buffer
size is the number of elements
Tries to read in all elements to buffer
*/
int readFile(int size, char buffer[size]){
    FILE* f = fopen(DISK_SWAP_FILE_PATH, "r");
    int r=fread(buffer, 1, size, f);
    fclose(f);
    return r;   
}
/*
Tests to make sure that swapout can write to correct portion of file
Tests to make sure that swapout can override values correctly
*/
void testSwapOut(){
    printf("RUNNING TESTS ON SWAP OUT FUNCTION:\n");
    char* mem = Memsim_GetPhysMem();
    for(int i = 0; i < 64; i++) mem[i] = i;
    for(int i = 0; i < 12; i++){
        swapOutFrame((i%4)*PAGE_SIZE, i);
    }
    char buff[192];
    readFile(192, buff);
    int pass = 1;
    for(int i = 0; i < 192; i++){
        if(buff[i]!=mem[i%64]){
            pass = 0;
            break;
        }
    }
    if(pass){
        printf("TEST 1: FILL FILE: PASSED\n");
    }
    else{
        printf("TEST 1: FILL FILE: FAILED\n");
    }
    for(int i = 0; i < 12; i++){
        swapOutFrame((3-(i%4))*PAGE_SIZE, i);
        
    }
    readFile(192, buff);
    pass = 1;
    // for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buff[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }

    for(int i = 0; i < 12; i++){
        for(int j = 0; j < 16; j++){
            if(buff[(i*PAGE_SIZE)+j]!=mem[((3-(i%4))*PAGE_SIZE)+j]){
            pass = 0;
            printf("Byte %d, Calculated Memory adress %d: %d, %d not equal!\n", (i*PAGE_SIZE)+j, ((3-(i%4))*PAGE_SIZE)+j, buff[(i*PAGE_SIZE)+j], mem[((3-(i%4))*PAGE_SIZE)+j]);
            break;
            }
        }
        if(!pass){
            break;
        }
    }
    if(pass){
        printf("TEST 2: OVERRIDE FILE: PASSED\n");
    }
    else{
        printf("TEST 2: OVERRIDE FILE: FAILED\n");
    }
}

/*
Tests to make sure that swapin can write correctly to memory and to disk 
Also tests to make sure that swapin exchanges the value in the disk and memory
*/
void testSwapIn(){
    printf("RUNNING TESTS ON SWAP IN FUNCTION:\n");
    char* mem = Memsim_GetPhysMem();
    for(int i = 0; i < 64; i++) mem[i] = i;
    for(int i = 0; i < 4; i++){
        swapOutFrame((i%4)*PAGE_SIZE, i);
    }
    char buff[192];
    readFile(192, buff);
    int pass = 1;
    for(int i = 0; i < 64; i++){
        if(buff[i]!=i && mem[i]!=0){
            pass = 0;
            break;
        }
    }
    if(pass){
        printf("TEST 1: SWAP IN FOUR PAGES: PASSED\n");
    }
    else{
        printf("TEST 1: SWAP IN FOUR PAGES: FAILED\n");
    }
    // for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buff[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }

}
/*
Tests initializing page tables, adding PTE, and checking to see if page tables exist
All tests assume that eviction of pages will not be necessary
*/
void testBasicPageTable(){
    printf("TESTING BASIC TABLE FUNCTIONS\n");
    int pass = 1;
    for(int i = 0; i < 4; i++){
        if(PT_PageTableExists(i)){
            pass = 0;
            break;
        }
    }
    if(pass){
        printf("TEST 1: EXISTS CAN INDENTIFY WHEN TABLES DO NOT EXISTS: PASSED\n");
    }
    else{
        printf("TEST 1: EXISTS CAN INDENTIFY WHEN TABLES DO NOT EXISTS: FAILED\n");
    }
    pass = 1;
    int temp = Memsim_FirstFreePFN();
    for(int i = 0; i<4; i++){
        temp = PT_PageTableInit(i, temp);
        if(!PT_PageTableExists(i)){ 
            pass = 0;
            break;
        }
        
        //printf("TEMP: %d\n", temp);
    }
    //printMPT();
    if(pass){
        printf("TEST 2: CAN INITIALIZE ALL PAGE TABLES, NO EVICTIONS NEEDED: PASSED\n");
    }
    else{
        printf("TEST 2: CAN INITIALIZE ALL PAGE TABLES, NO EVICTIONS NEEDED: FAILED\n");
    }
    MMUInit(); 
    pass = 1;
    temp = Memsim_FirstFreePFN();
    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                pass = 0;
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
        }
        //printf("TEMP: %d\n", temp);
    }
    for(int i = 1; i < 4; i++){
        // printf("%d, %d\t",i,  PT_VPNtoPA(0, i));
        if(PT_VPNtoPA(0, i)!=i*16){
            pass = 0;
            break;
        }
    }
    //printf("\n");
    // unsigned char* mem = Memsim_GetPhysMem();
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // printMPT();
    if(pass){
        printf("TEST 3: CAN INITIALIZE PAGE TABLES AND 3 ENTRIES, NO EVICTIONS NEEDED: PASSED\n");
    }
    else{
        printf("TEST 3: CAN INITIALIZE PAGE TABLES AND 3 ENTRIES, NO EVICTIONS NEEDED: FAILED\n");
    }

}
/*

*/
void testBasicEviction(){
    printf("TESTING BASIC EVICTION\n");
    int temp = 0;
    temp = Memsim_FirstFreePFN();
    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                printf("ERROR IN MAKING TABLE\n");
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
            temp+=16;
        }
        //printf("TEMP: %d\n", temp);
    }
    char* mem = Memsim_GetPhysMem();
    //simulating the page table being evicted
    swapInFrame(0,0);
    modifyMPT(0, 4);
    //simulates page being moved in
    for(int i = 0; i < 16; i++){
        mem[i] = i;
    }
    //simulates frame 3 getting filled with data
    for(int i = 48; i < 64; i++){
        mem[i] = i;
    }
    //simulates frame 3 being evicted
    swapInFrame(48,1);
    //checking to see if update table function works
    modifyTableInSecondary(0, 3, 5, 0, 1, 1, 0);
    
    
    char buff[192];
    readFile(192, buff);
    //for debugging
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // printf("SECONDARY MEMORY\n");    
    // for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buff[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }

    int pass = 1;
    for(int i = 0; i < 64; i++){
        if(i < 16 && mem[i] != i){
            pass = 0;
            break;
        }
        else if(i > 16 && mem[i]!=0){
            pass = 0;
            break;
        }
    }
    for(int i = 0; i < 6; i++){   
        if(i%2 == 1 && buff[i]!=6){
            pass = 0;
            break;
        }
        else if(i == 0 && buff[i]!=1){
            pass = 0;
            break;
        }
        else if(i == 2 && buff[i]!=2){
            pass = 0;
            break;
        }
        else if(i == 4 && buff[i]!=5){
            pass = 0;
            break;
        } 
    }
    if(pass){
        printf("TEST 1: PAGE EVICTED, TABLE NOT IN MAIN MEMORY: PASSED\n");
    }
    else{
        printf("TEST 1:  PAGE EVICTED, TABLE NOT IN MAIN MEMORY: FAILED\n");
    }

    pass = 1;
    MMUInit(); 
    temp = Memsim_FirstFreePFN();
    for(int i = 0; i<4; i++){
        temp = PT_PageTableInit(i, temp);
        if(!PT_PageTableExists(i)){ 
            pass = 0;
            break;
        }
        
        //printf("TEMP: %d\n", temp);
    }
    if(temp != 0){
        pass = 0;
    }
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // printf("SECONDARY MEMORY\n");    
    // for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buff[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }
    // printMPT();
    if(pass){
        printf("TEST 2: CAN INITIALIZE ALL PAGE TABLES, EVICTIONS NEEDED: PASSED\n");
    }
    else{
        printf("TEST 2: CAN INITIALIZE ALL PAGE TABLES, EVICTIONS NEEDED: FAILED\n");
    }

    
    MMUInit(); 
    temp = Memsim_FirstFreePFN();
    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                printf("ERROR IN MAKING TABLE\n");
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
            temp+=16;
        }
        //printf("TEMP: %d\n", temp);
    }
    mem = Memsim_GetPhysMem();
    //simulating the page table being evicted
    swapInFrame(0,0);
    modifyMPT(0, 4);
    //simulates page being moved in
    readFile(192, buff);
    for(int i = 0; i < 16; i++){
        mem[i] = i;
    }
    //checking to see if update table function works
    int t1 = loadAndCheck(0, 0, 0, 0, 0);
    int t2 = loadAndCheck(0, 1, 0, 0, 0);
    int t3 = loadAndCheck(0, 2, 0, 0, 0);
    int t4 = loadAndCheck(0, 3, 0, 0, 0);
    
    //for debugging
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // printf("SECONDARY MEMORY\n");    
    // for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buff[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }

    pass = 1;
    for(int i = 0; i < 64; i++){
        if(i < 16 && mem[i] != i){
            pass = 0;
            break;
        }
        else if(i > 16 && mem[i]!=0){
            pass = 0;
            break;
        }
    }
    if(t1 != -1 || t2 != 1 || t3 != 2 || t4 != 3){
        pass = 0;
    }
    
    //printf("RESULTS: %d %d %d %d\n", t1, t2, t3, t4);

    if(pass){
        printf("TEST 3: CHECKING FRAME, TABLE NOT IN MAIN MEMORY: PASSED\n");
    }
    else{
        printf("TEST 3:  CHECKING FRAME, TABLE NOT IN MAIN MEMORY: FAILED\n");
    }

    MMUInit(); 
    temp = Memsim_FirstFreePFN();
    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                printf("ERROR IN MAKING TABLE\n");
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
            temp+=16;
        }
        //printf("TEMP: %d\n", temp);
    }
    mem = Memsim_GetPhysMem();
    //simulating the page table being evicted
    swapInFrame(0,0);
    modifyMPT(0, 4);
    //simulates page being moved in
    for(int i = 0; i < 16; i++){
        mem[i] = i;
    }
    //checking to see if update table function works
    t1 = loadAndCheck(0, 0, 1, 1, 0);
    t2 = loadAndCheck(0, 0, 2, 1, 0);
    t3 = loadAndCheck(0, 0, 3, 1, 0);
    
    //for debugging
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // printf("SECONDARY MEMORY\n");    
    // for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buff[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }

    pass = 1;
    for(int i = 0; i < 64; i++){
        if(i < 16 && mem[i] != i){
            pass = 0;
            break;
        }
        else if(i > 16 && mem[i]!=0){
            pass = 0;
            break;
        }
    }
    if(t1 != t2 || t2 != t3 || t3 != 1){
        pass = 0;
    }
    
    //printf("RESULTS: %d %d %d %d\n", t1, t2, t3, t4);

    if(pass){
        printf("TEST 4: CHECKING PROTECTION, TABLE NOT IN MAIN MEMORY: PASSED\n");
    }
    else{
        printf("TEST 4:  CHECKING PROTECTION, TABLE NOT IN MAIN MEMORY: FAILED\n");
    }


    MMUInit(); 
    temp = Memsim_FirstFreePFN();
    for(int i = 0; i<3; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                printf("ERROR IN MAKING TABLE\n");
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
            temp+=16;
        }
        //printf("TEMP: %d\n", temp);
    }
    mem = Memsim_GetPhysMem();
    //checking to see if update table function works
    t1 = belongsToTableEntry(0, 0);
    t2 = belongsToTableEntry(0, 1);
    t3 = belongsToTableEntry(0, 2);
    t4 = belongsToTableEntry(0, 3);
    //printf("RESULTS: %d %d %d %d\n", t1, t2, t3, t4);
    //for debugging
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // printf("SECONDARY MEMORY\n");    
    // for(int i = 0; i < SEC_STORE_SIZE; i++){
	// 	printf("%d ", buff[i]);
	// 	if((i+1)%PAGE_SIZE==0){
	// 		printf("\n");
	// 	}
	// }

    pass = 1;
    if(t1 != -1 || t2 != 1 || t3 != 2 || t4 != -1){
        pass = 0;
    }
    
    //printf("RESULTS: %d %d %d %d\n", t1, t2, t3, t4);

    if(pass){
        printf("TEST 5: CHECKING BELONGS TO TABLE, TABLE IN MAIN MEMORY: PASSED\n");
    }
    else{
        printf("TEST 5:  CHECKING BELONGS TO TABLE, TABLE IN MAIN MEMORY: FAILED\n");
    }

    MMUInit(); 
    temp = Memsim_FirstFreePFN();
    for(int i = 0; i<3; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                printf("ERROR IN MAKING TABLE\n");
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
            temp+=16;
        }
        //printf("TEMP: %d\n", temp);
    }
    mem = Memsim_GetPhysMem();
    swapInFrame(0,0);
    modifyMPT(0, 4);
    //simulates page being moved in
    for(int i = 0; i < 16; i++){
        mem[i] = i;
    }
    //checking to see if update table function works
    t1 = belongsToTableEntry(0, 0);
    t2 = belongsToTableEntry(0, 1);
    t3 = belongsToTableEntry(0, 2);
    t4 = belongsToTableEntry(0, 3);
    
    //printf("RESULTS: %d %d %d %d\n", t1, t2, t3, t4);

    pass = 1;
    if(t1 != -1 || t2 != 1 || t3 != 2 || t4 != -1){
        pass = 0;
    }
    
    //printf("RESULTS: %d %d %d %d\n", t1, t2, t3, t4);    

    if(pass){
        printf("TEST 6: CHECKING BELONGS TO TABLE, TABLE NOT IN MAIN MEMORY: PASSED\n");
    }
    else{
        printf("TEST 6:  CHECKING BELONGS TO TABLE, TABLE NOT IN MAIN MEMORY: FAILED\n");
    }

}


void testMoreComplexEviction(){
    printf("TESTING MORE COMPLICATED EVICTIONS\n");
    int pass = 1;
    int temp = Memsim_FirstFreePFN();
    char *mem = Memsim_GetPhysMem();
    for(int i = 16; i < 64; i++){
        mem[i] = i;
    }

    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                pass = 0;
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
        }
        //printf("TEMP: %d\n", temp);
    }
    int i1 = PT_Evict_And_Replace(0);
    int i2 = PT_Evict_And_Replace(8);
    int i3 = PT_Evict_And_Replace(9);
    int i4 = PT_Evict_And_Replace(10);

    // char buff[192];
    
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    if(i1 != 0 && i2!= 1 && i3 != 2 && i4 != 3){
        pass = 0;
    }
     if(pass){
        printf("TEST 1: LOAD TABLE FULL OF ENTRIES, EVICT EVERYTHING: PASSED\n");
    }
    else{
        printf("TEST 1:  LOAD TABLE FULL OF ENTRIES, EVICT EVERYTHING: FAILED\n");
    }

    MMUInit();
    pass = 1;
    temp = Memsim_FirstFreePFN();
    mem = Memsim_GetPhysMem();
    for(int i = 16; i < 64; i++){
        mem[i] = i;
    }

    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                pass = 0;
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
        }
        //printf("TEMP: %d\n", temp);
    }
    i1 = PT_Evict_And_Replace(1);
    i2 = PT_GetRootPtrRegVal(0);
    i3 = PT_GetRootPtrRegVal(0);
    i4 = PT_Evict_And_Replace(9);
    int i5 = PT_Evict_And_Replace(10);

    char buff[192];
    
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    if(i2 != i3){
         pass = 0;
    }
     if(pass){
        printf("TEST 2: BRING BACK EVICTED TABLE: PASSED\n");
    }
    else{
        printf("TEST 2:  BRING BACK EVICTED TABLE: FAILED\n");
    }


    MMUInit();
    pass = 1;
    temp = Memsim_FirstFreePFN();
    mem = Memsim_GetPhysMem();
    for(int i = 16; i < 64; i++){
        mem[i] = i;
    }

    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                pass = 0;
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
        }
        //printf("TEMP: %d\n", temp);
    }
    i1 = PT_Evict_And_Replace(1);
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    i3 = PT_VPNtoPA(0, 1);
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    i3 = PT_GetRootPtrRegVal(0);
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    i4 = PT_VPNtoPA(0, 1);
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    
    if(mem[16] != 2 && mem[17] != 6 && mem[18]!=5 && mem[19]!=12 && mem[20]!=3 && mem[21]!=6){
        pass = 0;
    }
    i5 = PT_VPNtoPA(0, 2);
    
    if(pass){
        printf("TEST 3: VPN TO PA BASIC TEST: PASSED\n");
    }
    else{
        printf("TEST 3:  VPN TO PA BASIC TEST: FAILED\n");
    }

    MMUInit();
    pass = 1;
    temp = Memsim_FirstFreePFN();
    mem = Memsim_GetPhysMem();
    for(int i = 16; i < 64; i++){
        mem[i] = i;
    }

    for(int i = 0; i<4; i++){
        if(i == 0){
            //initilize page table
            temp = PT_PageTableInit(0, temp);
            if(!PT_PageTableExists(0)){
                pass = 0;
                break;
            }
        }
        else{
            //initilize page table entry
            PT_SetPTE(0, i, i, 0, 1, 1, 0);
        }
        //printf("TEMP: %d\n", temp);
    }
    i1 = PT_Evict_And_Replace(1);
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    i2 = PT_Evict_And_Replace(0);
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    i3 = PT_VPNtoPA(0, 1);
    // printf("PHYSICAL MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    
    if(mem[16] != 0 && mem[24] != 16 && mem[32]!=48){
        pass = 0;
    }
    // i5 = PT_VPNtoPA(0, 2);
    
    if(pass){
        printf("TEST 4: VPN TO PA MORE COMPLEX TEST: PASSED\n");
    }
    else{
        printf("TEST 4:  VPN TO PA MORE COMPLEX TEST: FAILED\n");
    }

}

void testInstructionMap(){
    // TOO COMPLICATED TO DESIGN TEST CASES FOR: CHECKING PRINT STATEMENTS MANNUALLY
    // Instruction_Map(0, 0, 1);
    // MMUInit();
    // Instruction_Map(0, 16, 1);
    // Instruction_Map(0, 32, 1);
    // Instruction_Map(0, 48, 1);
    // printf("\n");
    // printMPT();
    // printf("\n");
    // MMUInit();
    // Instruction_Map(0, 48, 1);
    // Instruction_Map(0, 16, 1);
    // Instruction_Map(0, 32, 1);
    // printf("\n");
    // printMPT();
    // printf("\n");
    // MMUInit();
    
    // printf("PID 1 ");
    // Instruction_Map(1, 16, 1);
    // printf("PID 0 ");
    // Instruction_Map(0, 16, 1);
    // printf("PID 0 ");
    // Instruction_Map(0, 32, 1);
    // printf("PID 1 ");
    // Instruction_Map(1, 32, 1);
    // printf("PID 1 ");
    // Instruction_Map(1, 48, 1);
    // printf("PID 0 ");
    // Instruction_Map(0, 48, 1);
    // printf("\n");
    // printMPT();
    // printf("\n");

    printf("PID 1 ");
    Instruction_Map(1, 16, 1);
    printf("PID 0 ");
    Instruction_Map(0, 16, 0);
    printf("PID 0 ");
    Instruction_Map(0, 32, 0);
    printf("PID 1 ");
    Instruction_Map(1, 32, 1);
    printf("PID 1 ");
    Instruction_Map(1, 48, 1);
    printf("PID 0 ");
    Instruction_Map(0, 48, 0);
    printf("PID 2 ");
    Instruction_Map(2, 16, 1);
    Instruction_Map(2, 32, 1);
    Instruction_Map(2, 48, 1);
    printf("PID 3 ");
    Instruction_Map(3, 16, 0);
    Instruction_Map(3, 32, 0);
    Instruction_Map(3, 48, 0);
    printf("PID 4 ");
    Instruction_Map(4, 16, 0);
    // printf("\n");
    // printMPT();
    // printf("\n");

    // char *mem = Memsim_GetPhysMem();
    // printf("PRIMARY MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%d ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    
    // char buff[192];
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%d ",buff[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }

}

void testInstructionStore(){
    // TOO COMPLICATED TO DESIGN TEST CASES FOR: CHECKING PRINT STATEMENTS MANNUALLY

    MMUInit();
    Instruction_Map(0, 16, 1);
    Instruction_Map(0, 32, 1);
    Instruction_Map(0, 48, 0);
    Instruction_Store(0, 33, 255);
    Instruction_Store(0, 49, 255);
    Instruction_Load(0,33);
    Instruction_Load(0,0);
    Instruction_Load(0,1);
    Instruction_Load(0,2);
    Instruction_Load(0,3);
    Instruction_Load(0,4);
    Instruction_Load(0,5);
    Instruction_Load(0,49);
    Instruction_Load(1,1);

    // unsigned char *mem = Memsim_GetPhysMem();
    // printf("PRIMARY MEMORY\n");
    // for(int i = 0; i < 64; i++){
    //     printf("%u ",mem[i]);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }
    
    // unsigned char buff[192];
    // readFile(192, buff);
    // printf("SECONDARY MEMORY\n");
    // for(int i = 0; i < 192; i++){
    //     printf("%u ",buff[i]&0xff);
    //     if((i+1)%16==0){
    //         printf("\n");
    //     }
    // }

    
}