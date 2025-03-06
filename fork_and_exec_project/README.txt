Important Information (please read contents of file BEFORE running any code):


1) Compiling:

There are two ways to compile the code so that it runs correctly. It can either be done via the makefile or by mannually compiling each file. 
Please note that in order for the makefile to work, the current file hierachy must be kept.

a) For the makefile:

Run the command: 

make all

To compile prolific.c, generation.c, explorer.c, slug.c, and slugrace.c into their respective exectuables

Run the command:

make clean

To remove the executables prolific, generation, explorer, slug, and slugrace


b) For mannual compiling:

i) Run the command for prolific:

gcc -Wall prolific.c -o prolific

ii) Run the command for Generation:

gcc -Wall generation.c -o generation

iii) Run the command for Explorer:

gcc -Wall explorer.c -o explorer

iv) Run the command for Slug:

gcc -Wall slug.c -o slug

v) Run the command for SlugRace:

gcc -Wall slugrace.c -o slugrace


2) Running the programs:

a) Running prolifc:

Type the command: 

./prolific


b) Running generation:

Type the command:

./generation

c) Running explorer:

Type the command:

./explorer

d) Running slug:

Type the command:

./slug num

Where:

num represents the number file that the seed is to be taken from (1-4)

e) Running slugrace (please note that the slug executable has to exist for this one to work):

Type the command:

./slugrace

Answer to Questions:

Scenerio 1 Design: The reason why the rand() function is called before the forking
rather than by the children is because of the fact that if the children call the rand()
function they will all receive the same values for the first, second, etc calls. Therefore
in order for them to wait different random intervals of time the rand() function must be called
beforehand.


Sceneraio 2 Design: If child processes did not wait for subchild processes then it could 
create zombie processes given that the only point of contact for the subchild process is through
the child process. When the child processes terminates it would cut the tie from the parent to the 
subchild process and like when the parent ends before the child, the result would be an 
unreachable process that just consumes resources but has no way of being contacted.


Sceneraio 3 Design: For the way I design programs I like to add in extra printf statements that are 
commented out as I develop the program. What I would do is have print statements before the change of directory
and after to confirm that the program indeed changed directories. Additionally, if the directory was correctly changed
but the program still produced incorrect responses I would navigate to the directory myself in order to run the command
to insure that the issue was with the command. If I have found that to be the case I would check to make sure the correct
command and location have been passed to the functions by adding print statements before each of the major calls:
including, but not limitted to the fork(), chdir(), and execvp().

Finally, if all else fails I could use the gdb debbugger tool with break points in order to see what is actually
being loaded into memory.


Sceneraio 4 Design: Diffrernt seed files are critical for running parallel processes in The Slug Race because of the fact
that it allows for the random number generators to generate different values for each of the slug child processes. With
only one file then the seed for each slug's random number generator would be the same resulting in the same generated values.
Thus, there must exist multiple seed files in order to allow for each slug to have a potentially unqiue seed value (of course
this isn't to say one could set all the seeds in each file to the same number).


Sceneraio 5 Design: If I were to implement this with more features I would probably add a feature that logs how far into
the slug process each slug so that instead of being able to display which slugs are still running I could display the percentage
of the work that the process that the slug has done. This would allow for me to then have a system that would display the process
with either a percentage or a progress bar next to it that are updated ever arbitrary time interval in order to give the user
the ability to understand how much longer each slug might be running.


EXTRA NOTE:

The slugrace uses signal SIGUSR1 in order for the parent to communicate with the children to get them to print messages at the correct times (and start all slugs at the "same" time)
Removing the signal and pause statements allow the code to execute correctly, however the order of print statements will be out of order