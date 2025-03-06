PLEASE NOTE:

1) Code from the files pagetable.c, instruction.c, and makefile are original code

2) The contents of other files were included as part of the starting code for the class project



Important Information (please read contents of file BEFORE running any code):


1) Compiling:

There are two ways to compile the code so that it runs correctly. It can either be done via the makefile or by mannually compiling each file. 
Please note that in order for the makefile to work, the current file hierachy must be kept.

a) For the makefile:

Run the command: 

make 

To compile mmu executable

Run the command:

make clean

To remove the executable mmu and binary files


b) For mannual compiling:

Run the commands for mmu:

gcc -c mmu.c -o mmu.o
gcc -c input.c -o input.o
gcc -c pagetable.c -o pagetable.o
gcc -c memsim.c -o memsim.o
gcc -c instruction.c -o instruction.o
gcc mmu.o input.o pagetable.o memsim.o instruction.o -o mmu


To run the executable mmu use the command:

./mmu < fileToRead > fileToOutput

where: 

fileToRead: the relative path for the file that contains the instructions
fileToOutput: the relative path for the file that will contain the output

General:

What does the this project do?

1) Takes in instructions of the form please note that the () are meant to suggest the meaning of the number:

#(pid),map,#(va),#(protection)

or

#(pid),store,#(va),#(value to store)

or

#(pid),load,#(va),NA

and using page tables in order to keep track of the simulated processes data.

2) Please note: This project assumes that there are at most 4 processes (process 0-3), and that each process has addresses in virtual memory between 0-63.


How does eviction work?

1) Since the disk space starts off zeroed out (by design), evicting a page (no replacement) will simply swap the values at the 
first free frame in secondary memory (disk.txt) to that of the frame to be evicted. 

2) Evict and replace will do the same except for the frame from secondary memory is inputted as part of the funciton.

3) The algorithm for determing the frame to evict is a modified round robin algorithm

4) The modifications are that it checks the frame to make sure it is either a:

i) Page 

ii) Page table which has no entries in main memory, and does not belong to the evicting process.

5) Note: there is an array that keeps track of which process "owns" a given frame in main memory.


What is the address system?

In this project I assume that the physical address space is from 0-255, in which 0-63 is in main memory, and 64-255 is in secondary memory.
It is assumed that each page table can only have virtual pages numbered 0-3.


How is the master page table configured?

The master page table stores the frame number for the start of each page table, whether it is in main memory, whether it is in secondary memory,
and a bitmap that tracks which pages have been initialized.


How is each page table entry configured?

1) Each page table entry contains 5 bits to represent the frame number it is mapped to, 1 bit to represent whether the page has been initialized,
and 1 bit to represent it has write permission (1 means it does).

2) All pages are assumed to have read permission as a given.
