#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define NUMSTRINGS 20
#define STRINGLEN 256


//Prototypes 
void store_words_from_file(char* fileName); 
bool isPalindrome(char str[]);


int main(int argc, char *argv[]) {
	char* fileName;

	//Check that fileName was given 
	if(argc < 2) { 
		printf("\nERROR: No fileName given for palin.c");
		return 0;
	}
	else { 
		fileName = argv[1];	
		store_words_from_file(fileName);
	}
	return 0;
}

void store_words_from_file(char* fileName) { 
	//Open file
	FILE *fptr; 
	if((fptr = fopen(fileName, "r")) == NULL) { 
		perror("\nERROR: Could not read file");
		return;
	}
	

	//Array to store each string
	char words[NUMSTRINGS][STRINGLEN]; //Assuming 20 words in file with length 256 max

	//Index to words array 
	int i = 0;

	//Read strings line by line from file and store into words array
	while(fgets(words[i], STRINGLEN, fptr) != NULL){
		printf("String read is :\n%s", words[i]);
		
		//Define terminating character 
		words[i][strlen(words[i]) -1] = '\0';
		i++; //increase index position by 1		
	}


	//Code to go into each position of word array one by one
	int total_strings = i;	
	int index = 0;	
	while(index < total_strings){
		printf("\nWord is :%s", words[index]);
		printf("\nIs palindrome: %d", isPalindrome(words[index]));
		index++;
	}

	
	fclose(fptr);
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
			printf("\n%s is Not Palindrome", str);
			return false;
		}
	}
	return true;
}
