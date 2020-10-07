#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <time.h>

#define SMSIZE 100
#define PERMS (S_IRUSR | S_IWUSR)

//Prototypes
void read_the_clock();

//Globals
struct shared_memory_data{
	int process_group_id;
};

struct shared_memory_data *shm_data;


/*-----------------------------MAIN-----------------------------*/
/*--------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	key_t key; 
	int shmid;
	int shmdetach;
	int shmcontrol; 
	pid_t pid;


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
	shmid = shmget(key, SMSIZE, PERMS | IPC_CREAT);  
	//If creation of shared memory segment fails then return with error message 
	if(shmid == -1) { 
		perror("\nERROR: Could not create shared memory segment"); 
		return 1;	
	}	

	//Use shmat to attach shmid's shared memory segment to the address space of the calling process 
	shm_data = (struct shared_memory_data*) shmat(shmid, NULL, 0); 
	if (shm_data == (struct shared_memory_data*)(-1)) { 
		perror("\nERROR: Could not attach shared memory");
		return 1;
	}
	

	//Captures user input 
	int opt; 
	int max_total_cp = 4;
	int concurr_children = 2;
	int forced_time_quit = 100;

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
			break; 
		
			//Indicate the number of chilren allowed to exist in the system at the same time
			case 's':
				concurr_children = atoi(optarg); 
			break;
			
			//The time in seconds after which the process will termiante, even if it has not finished
			case 't': 
				forced_time_quit = atoi(optarg);
			break;			 					
			
			//Invalid argument case
			default: 
				perror("\nERROR: Invalid option");
				return 1;
			break;
		}
	}


	//Create child process
	pid = fork(); 
	//If failed to create a new process then return with error message
	if(pid == - 1) {
		perror("\nERROR: Could not create child process"); 
		return 1;
	}
	else if(pid == 0) {
		//Print processes id's
		printf("\nChild process id is %u\n", getpid());
		printf("\nParent of child process id is %u\n", getppid());
		printf("\nChild process will read the clock\n");
		read_the_clock();
	}
	else { 
		//A positive number is returned for pid of parent process
		//Print processes id's
		printf("\nParent of parent process is %u\n", getppid());
		printf("\nParent process is %u\n", getpid());
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
	
	//Detach from the memory segment 
	shmdetach = shmdt(shm_data);
	//If detachment fails then return with error message
	if(shmdetach == -1) { 
		perror("\nERROR: Could not detach memory segment"); 
		return 1;
	}	
	//Remove memory segment 
	shmcontrol = shmctl(shm_data);
	//If removal of shared memory fails then return with error message 
	if(shmcontrol == -1) { 
		perror("\nERROR: Could not remove shared memory segment"); 
		return 1;
	}
	
	return 0;
}

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
	minutes = local->tm_min; 	//get minutes passed after the hour (0-59)
	seconds = local->tm_sec;	//get seconds passed after minute (0-59)

	//print local time 
	if(hours < 12) {	//before midday 
		printf("Time is : %02d:%02d:%02d am\n", hours, minutes, seconds);
	}
	else {			//after midday 
		printf("Time is : %02d:%02d:%02d pm\n", hours -12, minutes, seconds);
	}
}
