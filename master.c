#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>

/*------------------------------------------------GLOBALS AND PROTOTYPES-------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/

#define PERMS (S_IRUSR | S_IWUSR)	//Shared memory permissions
#define MAXPROCESSESCAP 20;		//Cap on amount of child processes in the system at any given time

//Prototypes
void read_the_clock();
int free_memory();
void sighandler(int);
void forkMultipleProcesses();

//Shared memory global
typedef struct{
	pid_t pgid;
}shared_memory_data;

shared_memory_data *shm_data_ptr;

//Memory globals
shared_memory_data *shm_data_ptr;	//shared memory data pointer
int shmid;				//shared memory segment id
pid_t pid;				//child process


//Default arguments if no user input
int max_total_cp = 4;
int concurr_children = 2;
int forced_time_quit = 100;






/*------------------------------------------------------MAIN-----------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	//library function to set a function to handle signal
	signal(SIGINT, sighandler);
	
	//memory variables
	int key; 
	int shmid;


	//Use ftok to convert pathname and id value 'a' to System V IPC key 
	key = ftok("./master", 'a'); 	
	//If key generation fails then return with error message
	if(key == -1) { 
		perror("\nERROR: Could not generate key"); 
		return 1;
	}

	//Use shmget which allocates a shared memory segment and returns identifier of System V shared memory segmentassociated with the value of the argument key
	//Store shared memory segment identifier into shmid 
	//Sets permissions, creates a new segment, and fails if segment already exists
	shmid = shmget(key, sizeof(shared_memory_data), PERMS | IPC_CREAT);  
	//If creation of shared memory segment fails then return with error message 
	if(shmid == -1) { 
		perror("\nERROR: Could not create shared memory segment"); 
		return 1;	
	}	

	//Use shmat to attach shmid's shared memory segment to the address space of the calling process 
	shm_data_ptr = (shared_memory_data*) shmat(shmid, NULL, 0); 
	if (shm_data_ptr == (void*)-1) { 
		perror("\nERROR: Could not attach shared memory");
		return 1;
	}
	

	//Captures user input 
	int opt;


	//Use getopt funct to parse arguments
	while((opt = getopt(argc, argv, "hn:s:t:")) !=-1){
		switch(opt){
			//Help. Show details
			case 'h':
				printf("\n master should take in several command line options as follows:\n"); 
				printf("\n master -h\n");
				printf("\n master [-n x] [-s x] [-t time] infile\n");
				printf("\n -h		Describe how the project should be run and then, terminate.\n"); 
				printf("\n -n x		Indicate the maximum total child processes master will ever create (Default 4)\n"); 
				printf("\n -s x		Indicate the number of children allowed to exist in the system at the same time (Default 2)\n"); 
				printf("\n -t time	The time in seconds after which the process will terminate, even if it has not finished. (Default 100)\n"); 
				printf("\n infile		Input file containing strings to be tested.\n"); 				
				return 0;

			break;
		
			//Indicate the maximum total of child processes master will ever create
			case 'n': 
				max_total_cp = atoi(optarg);
				if(max_total_cp > 20 || max_total_cp <= 0) {
					printf("\nERROR: Total processes in system must be between 1-20\n");
					return 1;
				}
			break; 
		
			//Indicate the number of chilren allowed to exist in the system at the same time
			case 's':
				concurr_children = atoi(optarg); 
				if(concurr_children <= 0) { 
					printf("\nERROR: There must be at least 1 concurrent process\n");
					return 1;
				}
			break;
			
			//The time in seconds after which the process will termiante, even if it has not finished
			case 't': 
				forced_time_quit = atoi(optarg);
				if(forced_time_quit <= 0) { 
					printf("\nERROR: Time must be at least 1\n");
					return 1;
				}
			break;			 					
			
			//Invalid argument case
			default: 
				perror("\nERROR: Invalid option. Enter -h for help");
				return 1;
			break;
		}
	}


	//Fork multiple processes until maximum limit reached
	int fork_number;
	while(fork_number <= max_total_cp){
		printf("\nFork count :%d\n", fork_number);
		forkMultipleProcesses(fork_number++);
	}


	//Capture input file containing strings to be tested
	char* fileName = argv[optind];
	
	//Open the file
	FILE *fp = fopen(fileName, "r");

	//If file cannot be opened then return
	if(fp ==0) { 
		perror("\nERROR: Could not open file");
		return 1;
	}

	//If file can be opened, then continue program
	else { 


/*--------------------------------EXEC PALIN------------------------------------*/
/*------------------------------------------------------------------------------*/	
	//Send to function the check for palindromes 
	//execl("./palin", "palin", fileName, NULL);


	}
	
	return 0;	
}





/*--------------------------------------------------FUNCTIONS--------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/



//Algorithm for fetching current time is from techiedelight tutorial
void read_the_clock() {
	//Variable to store time component
	int hours, minutes, seconds;
	//time_t is arithmetic time type
	time_t now;
	
	//Obtain current time 
	//time() returns the current time of the system as a time_t value
	time(&now);	
	
	//localtime converts a time_t value to calendar time and returns a pointer to a tm structure with its members
	//fille with the corresponding values 
	struct tm *local = localtime(&now);

	hours = local->tm_hour; 	//get hours since midnight(0-23)
	minutes = local->tm_min;	//get minutes passed after the hour (0-59)
	seconds = local->tm_sec;	//get seconds passed after minute (0-59)

	//print local time 
	if(hours < 12){		//before midday
		printf("Time is : %02d:%02d:%02d am\n", hours, minutes, seconds); 
	}
	else if(hours == 12){
		printf("Time is : %02d:%02d:%02d pm\n", hours, minutes, seconds);
	}
	else {			//after midday 
		printf("Time is : %02d:%02d:%02d pm\n", hours -12, minutes, seconds);
	}

}

int free_memory(){
	//Detach from the memory segment
	int shmdetach = shmdt(shm_data_ptr);
	//If detachment fails then return with error message 
	if(shmdetach == -1) { 
		perror("\nERROR: Could not detach memory segment");
		return 1;
	}
	//Remove memory segment 
	int shmcontrol = shmctl(shmid, IPC_RMID, NULL);
	//If removal of shared memory fails then return with error message
	if(shmcontrol == -1) { 
		perror("\nERROR: Could not remove shared memory segment");
		return 1;
	}
}


void sighandler(int signal){
	killpg(shm_data_ptr->pgid, SIGTERM);
	printf("\nTime children processes killed:");
	read_the_clock();

	int status;
	if(waitpid(pid, &status, 0) != -1) { 
		if(WIFEXITED(status)){
			int returned = WEXITSTATUS(status);
			printf("\nExited normally with status %d\n", returned);
		}
		else if(WIFSIGNALED(status)){
			int signum = WTERMSIG(status);
			printf("Exited due to receiving signal %d\n", signum);
		}
		else if(WIFSTOPPED(status)){
			int signum = WSTOPSIG(status);
			printf("Stopped due to receiving signal %d\n", signum);
		}
		else { 
			printf("Something strange happened\n");
		}
	}
	else { 
		perror("waitpid() failed");
	}

	free_memory();
	exit(0);
	
}

void forkMultipleProcesses(int fork_number){
	
	//Create child process
	pid = fork(); 
	
	//If child was created
	if(pid == 0) {
		//Print processes id's
		printf("\nChild process id is %u\n", getpid());
		//The first fork sets the process_group_id where all other processes will be associated
		if(fork_number ==0){
			shm_data_ptr->pgid = getpid();
		}
		//Group processes together
		else{
			setpgid(0, shm_data_ptr->pgid);
		}
		printf("\n Stored process_group_id as :%d", shm_data_ptr->pgid);
		printf("\nParent of child process id is %u\n", getppid());
		printf("\nChild process will read the clock\n");
		read_the_clock();
		sleep(1);
		//while(1);	//go into infinite loop to test parent termination
	}
	//If failed to create a new process then return with error message
	else if(pid == - 1) {
		perror("\nERROR: Could not create child process"); 
	}
	//Main (parent) process after fork succeeds
	else { 
		int returnStatus;
		waitpid(pid, &returnStatus, 0);	//Parent process waits here for child to terminate

		//Print process id's		
		//printf("\nParent of parent process is %u\n", getppid());
		//printf("\nParent process is %u\n", getpid());
	
		printf("\nParent will read the clock\n");
		read_the_clock();	
	}

}	
