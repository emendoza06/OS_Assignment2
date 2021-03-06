#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <getopt.h>
#include <time.h>


/*-------------------------------------------------GLOBALS AND PROTOTYPES-------------------------------*/
/*------------------------------------------------------------------------------------------------------*/

#define PERMS (S_IRUSR | S_IWUSR)	//Shared memory permissions


//Prototypes 
bool isPalindrome(char str[]);
char* readTheClock();


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




/*---------------------------------------------------------MAIN-----------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/


int main(int argc, char *argv[]) {

	//memory variables 
	int key; 
	int shmid;

	//Use ftok to convert pathname and id value 'a' to System V IPC key
	//Will give the same key used for master
	key = ftok("./master", 'a');	
	//If key generation fails then return with error message
	if(key == -1){
		perror("\nERROR: Could not generate key");
		return 1;
	}

	//Store shared memory segment identifier
	shmid = shmget(key, sizeof(shared_memory_data), PERMS | IPC_CREAT);
	if(shmid == -1) {
		perror("\nERROR: Could not find shared memory segment");
		return 1;
	}

	//Attach to shared memory segment
	shm_data_ptr = (shared_memory_data*) shmat(shmid, NULL, 0);
	if(shm_data_ptr == (void*)-1){
		perror("\nERROR: Could not attach shared memory");
		return 1;
	}


	//Save argument as index of child
	int index;	
	index = atoi(argv[1]);	

	//Each child reads a string from file at index 
	bool palin = isPalindrome(shm_data_ptr->words[index]);

	//Waiting room for Critical Section; sourced from SOlution 4 in notes
	printf("\nID %d PID %d wants to access critical section", index, getpid());
	int N = shm_data_ptr->children;	//number of processes
	enum state {idle, want_in, in_cs};
	int j; 
	do{
		shm_data_ptr->flags[index] = want_in;
		j = shm_data_ptr->turn;
		while(j != index){
			j = (shm_data_ptr->flags[j] != idle) ? shm_data_ptr->turn : (j + 1) % N;
		}

		shm_data_ptr->flags[index] = in_cs;

		for(j = 0; j < N; j++) { 
			if(j != index && shm_data_ptr->flags[j] == in_cs)
				break;
		}
	} while((j < N) || ((shm_data_ptr->turn != index) && (shm_data_ptr->flags[shm_data_ptr->turn] != idle)));
	shm_data_ptr->turn = index;
		
	

	//Sleep for 1 seconds 
	sleep(1);

	//Critical section 
	printf("\nID %d PID %d is in critical section", index, getpid());
	//Write to palin and no palin
	//If a palindrome then go to palin else nopalin
	FILE *palinFile = fopen(palin ? "palin.out" : "nopalin.out", "a+");	//append
	
	//Error if can't open file
	if(palinFile == NULL) {
		perror("\nERROR: Could not create file");
		return 1;
	}
	//print the string to file
	fprintf(palinFile, "%s\n", shm_data_ptr->words[index]);
	fclose(palinFile);

	//Write to log file
	FILE *logFile = fopen("output.log", "a+");
	//Error if can't open file
	if(logFile == NULL) {
		perror("\nERROR: Could not open file");
		return 1;
	}

	time_t mytime = time(NULL);
	char *time_str = ctime(&mytime);
	time_str[strlen(time_str)-1] = '\0';

	fprintf(logFile, "%s %d %d %s\n", time_str, getpid(), index, shm_data_ptr->words[index]);
	fclose(logFile);


	return 0;
}




//Algorithm sourced from geeksforgeeks to check if a string is a palindrome
//Does not ignore upper-lower case values
bool isPalindrome(char str[]) {

	//Start from leftmost and rightmost corners of str 
	int l = 0; 
	int h = strlen(str) - 1; 

	//Keep comparing characters while they are the same 
	while (h > l) { 
		//If characters are not the same
		if(str[l++] != str[h--]) { 
		//	printf("\n%s is Not Palindrome", str);
			return false;
		}
	}
	return true;
}



//Algorithm for fetching current time is from techiedelight tutorial
char* readTheClock() {
	char *time_result;
	
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
		snprintf(time_result, sizeof(time_result), "Time is: %02d:%02d:%02d am\n", hours, minutes, seconds);
//		time_result = "Time is : %02d:%02d:%02d am\n", hours, minutes, seconds;
	}
	else if(hours == 12){
		snprintf(time_result, sizeof(time_result), "Time is: %02d:%02d:%02d pm\n", hours, minutes, seconds);
//		time_result = "Time is : %02d:%02d:%02d pm\n", hours, minutes, seconds;
	}
	else {			//after midday 	
		snprintf(time_result, sizeof(time_result), "Time is: %02d:%02d:%02d pm\n", hours, minutes, seconds);
//		time_result = "Time is : %02d:%02d:%02d pm\n", hours -12, minutes, seconds;
	}

	
	return time_result;

}
