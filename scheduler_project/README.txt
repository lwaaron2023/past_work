Important Information (please read contents of file BEFORE running any code):


1) Compiling:

There are two ways to compile the code so that it runs correctly. It can either be done via the makefile or by mannually compiling each file. 
Please note that in order for the makefile to work, the current file hierachy must be kept.

a) For the makefile:

Run the command: 

make 

To compile scheduler executable

Run the command:

make clean

To remove the executables simulation


b) For mannual compiling:

i) Run the commands for simulation:

gcc -g -Wall -c scheduler.c
gcc -g -Wall -c queue.c
gcc -g -Wall scheduler.o queue.o -lm -o scheduler

a) Running scheduler:

Type the command: 

./scheduler Pol FL TS

where:

Pol: the policy to be used (either FIFO, SJF, RR)
FL: the location of the test file (parent folder/file), assumes that the parent folder is in the project folder
TS: the time slice to be used (always include, not always used)


Answer to Questions

Data structures used:

I used a queue, node, and job data structure. The queue consists of a start and end node (can be the same one) and a integer
to represent its size. The job data structure consists of integers representing: its id, length, start time, end time, times run,
and the total amount of time it waited. The node data structure consists of a pointer to a job and a pointer to the next node.

Algorithms used:

To order the jobs by length in the queue for the SJF policy I used an instertion sort algorithm.
To order the jobs by arrival time I used selection sort for the RR policy analysis section.


Reasoning for chosen workloads:

1) In order for wait time to be equal to the response time, the time for each job must be less than or equal to that 
of the timeslice.

2) In order for the FIFO turnaround time to be 10 times greater than that of the SJF approach there must be 10 jobs, 
in which the first job (in the order presented) is significantly larger than all other jobs. This is because the average turnaround
is for any non-preemptive policy for 10 jobs calculated by the equation (10*first+9*second+...+last)/10. 
Therefore a large enough first value im the order presented will cause the FIFO approach to be around 10 times larger due to the 
first value presented being the first job for the FIFO while the shortest is the first job of the SJF. Thus, presenting a large value
first followed by small values will cause the cofficient to add significantly more to the average for the FIFO approach. 

3) In order for the average wait time and response time for the three methods to be equal, all the jobs must have equal lengths.
This is because the SJF approach will choose the shortest job, meaning that any difference in job length will cause a difference in wait and
response times. The jobs also must be less than or equal to the time slice because if the jobs are greater than the timeslice
the wait time for each job will not be equal to the lengths of the previous jobs.

4) Given that the last job length does not contribute to its wait time, having small job lengths for the first 4 jobs will keep the wait
time under 5 seconds average. Then to get a large average throughput, the job length for the last job can be very large because
the job length of the last job does effect the average throughput.

5) In order for the jobs to have an average throughput of 13 then the total throughput must be equal to 39. In order for the 
jobs to have an average response time of 5 then the total response time is equal to 15. Given that job 1 is 3 in length, then 
the equation for total response time 2*j1+j2=15 can be solved by plugging in 3 for j1. This yields a job 2 length of 9. Then these 
two values can be plugged into the equation for total throughput 3*j1+2*j2+j3 = 39 to get a value of 12 for job 3.