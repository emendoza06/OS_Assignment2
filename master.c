/* NAME: Epharra Mendoza
 * DATE: 10/7/2020
 * ASSIGNMENT 2 OPERATING SYTEMS
 * CS 4760
 * */

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
#include <string.h>
#include <getopt.h>

/*------------------------------------------------GLOBALS AND PROTOTYPES-------------------------------*/
/*-----------------------------------------------------------------------------------------------------*/

#define PERMS (S_IRUSR | S_IWUSR)	//Shared memory permissions
#define MAXPROCESSESCAP 20		//Cap on amount of child processes in the system at any given time



//Prototypes
void readTheClock();
int free_memory();
void sighandler(int);
void forkMultipleProcesses();
void storeWordsFromFile(char* fileName);

//Shared memory global
typedef struct{
	int children;
	char words[20][256];	//Assuming 20 words in file with length 256 max	
	pid_t pgid;
	int id;
	int flags[20];
	int turn;
}shared_memory_data;


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
//	signal(SIGALRM, exitfunc);
	
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
	
	
	//Capture input file containing strings to be tested
	char* fileName = argv[optind];
		
	//Timer code
/*	int sec = 0, trigger = forced_time_quit;
	clock_t before = clock();

	int doing = 0;
	do{ 
		if(doing == 0) { 
			storeWordsFromFile(fileName);
			doing = 1;
		}
		clock_t difference = clock() - before;
		sec = difference * 10 / CLOCKS_PER_SEC;

	}while( sec < trigger);	
	
	printf("\nTime's up");
	killpg(shm_data_ptr->pgid, SIGTERM);
	free_memory();*/
		

	storeWordsFromFile(fileName);


	
	return 0;	
}





/*--------------------------------------------------FUNCTIONS--------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/




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
		//The first fork sets the process_group_id where all other processes will be associated
		if(fork_number ==0){
			shm_data_ptr->pgid = getpid();
		}
		//Group processes together
		else{
			setpgid(0, shm_data_ptr->pgid);
		}
		
		sleep(1);
		
		//Before passing id to palin, need to convert argument to a string
		char argument[256];
		sprintf(argument, "%d", fork_number);
		
		execl("./palin", "palin", argument, NULL);
	
	}

}


void storeWordsFromFile(char *fileName){
	//Open file
	FILE *fptr;
	if((fptr = fopen(fileName, "r")) == NULL){
		perror("\nERROR: Could not read file");
		return;
	}

	//Index to words array
	int i = 0; 

	//Read strigs line by line from file and store into words array
	while(fgets(shm_data_ptr->words[i], 256, fptr) != NULL){
		printf("\nString read is:\n%s",shm_data_ptr->words[i]);

		//Define terminating character
		shm_data_ptr->words[i][strlen(shm_data_ptr->words[i]) -1] ='\0';
		i++; //Increase index position by 1
	}

	//Code to go into each position of word array one by one 
	//Each child process gets its own string, but there is a maximum limit to the amount of processes in system
	int total_strings = max_total_cp;
	int children = max_total_cp;
	int index = 0;
	while(index < total_strings){
		//Assign an id to each child
		shm_data_ptr->id = index;

		forkMultipleProcesses(shm_data_ptr->id);
		index++;
	}

	fclose(fptr); 
}	
