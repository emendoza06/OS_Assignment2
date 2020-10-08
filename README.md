#Description: 
Main program reads from a list of palindromes from a file (one strig per line) into shared memory and forks off processes. Each child tests the string at the index assigned to it and writes the string into the appropriate file named palin.out and nopalin.out. In addition, processes writes into a log file containing PID Index String. 

The code for each child processes uses the following template: 
	determine if the string is a palindrome; 
	execute code to enter critical section;
	sleep for random amount of time (between 0 and 2 seconds); 
	/* Critical section */
	execute code to exit from critical section; 
	

#Invoking the solution 
master will take the command line options: 
master -h 
master [-n x] [-s x] [-t time] infile

Run the following Makefile arguments: 
make clean - removes built objects 
make	- updates targets master and palin 

#Outstanding Issues
Program will accept time as an argument, but will not terminate when given time has elapsed. Section of code has been commented out. 

Program can read all strings in file, and can recognize if they are palin or no palin but it will not always print all of these strings to the necessary files. 

Program does not always print when child is entering or wants to enter critical section
