// Starting code version 1.1

/*
 * Public Interface:
 */

#define NUM_PROCESSES 4
#define SEC_STORE_SIZE 256

/*
 * Public Interface:
 */
// Starting code version 1.1


int PT_SetPTE(int pid, int VPN, int PFN, int valid, int protection);
void PT_UpdatePTE(int pid, int VPN, int PFN);
int PT_PageTableInit(int pid, int pa);
int PT_PageTableExists(int pid);
int PT_GetRootPtrRegVal(int pid);
int PT_Evict(int pid);
int PT_Evict_And_Replace(int pid, int frameNum);
int PT_PAtoVPN(int pid, int frameNum);
int PT_VPNtoPA(int pid, int VPN);
int PT_PIDHasWritePerm(int pid, int VPN);
void PT_Init();
void swapInFrame(int addr, int secF);
int existsVPN(int pid, int VPN);
void printMPT();
void printMainMem();
void printSecondMem();
void findFrameToEvict(int pid);
