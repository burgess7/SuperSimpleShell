#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "utils.h"

// function to turn input line into dynamically allocated arg. array;
// returns 0 for successful exec() call, 1 for failed exec(), and 2 for exit
int execLine(char* lineIn);

int main (int argc, char* argv[]) {

	// user mode; loop until exit requested
	if(argc == 1){
		int childExit = 0;

			// command execution loop, runs at least once until exit
			do{

			pid_t pid = fork();

			//child process prompts user, trims and sends input to execLine
			if(pid == 0){
				printf("> ");

				char* line = NULL;
				size_t len = 0;

				// take in entire input line, get rid of \n
				getline(&line, &len, stdin);
				line[strlen(line) - 1] = '\0';

				// parse and execute the input line; return exit code
				childExit = execLine(line);
				exit(childExit);
			}

			// parent process waits for the child to terminate, uses its
			// exit code to decide what's next
			else{
				int stat;
				wait(&stat);
				int exit = WEXITSTATUS(stat);

				childExit = exit;
				if(exit == 1){
					printf("Command failed. Please try again.\n");
				}
			}

		// keep looping until exit condition
		}while(childExit != 2);

		exit(0);
	}

	// File mode: loop execution until EOF or invalid command reached
	else if(argc == 2){

		const char* title = argv[1];
		FILE *input = fopen(title, "r");

		char* line = NULL;
		size_t sz = 0;
		ssize_t getlineSuccess = 0;
		getlineSuccess = getline(&line, &sz, input);
		line[strlen(line) - 1] = '\0';

		int stat = 0;

		// keep reading in lines until EOF reached
		while((int) getlineSuccess != -1){
			pid_t pid = fork();

			// child process tries to execute line. Closes the file if it
			// fails, because a failed exec() will not close the file
			if(pid == 0){
				int ret = execLine(line);
				if(ret == 1){
					fclose(input);
				}
				exit(ret);
			}

			// parent process again waits for child, acts on return value
			else{
				wait(&stat);
				int exitStat = WEXITSTATUS(stat);

				// process exits with code 1 as soon as an exec() call fails
				if(exitStat == 1){
					printf("Invalid command read. Terminating execution\n");

					// free child process' memory (same reason as above)
					free(line);
					fclose(input);
					exit(1);
					getlineSuccess = -1;
				}

				// otherwise, keep looping until getline fails (EOF)
				else{
					getlineSuccess = getline(&line, &sz, input);
					line[strlen(line) - 1] = '\0';
				}

			}

		}

		// if we get here, we've reached EOF with no error. Clean up and exit.
		free(line);
		fclose(input);
		exit(0);

	}


	// >1 argument -> error, quit with code 1
	else{
		fprintf(stderr, "Error: shell must be run with 0 or 1 args.");
		exit(1);
	}

	return 0;

}


int execLine(char* lineIn){

	// start by allocating array of size 4 (realloc as needed)
	char** tokenArr = malloc(4 * sizeof(*tokenArr));

	// idea is to whittle down lineIn one arg at a time, hence "remainder"
	char* remainder = lineIn;

	int firstSpace = first_unquoted_space(lineIn);

	// ints to keep track of array size/position; currently size 4, starting
	// at index 0
	int indexCount = 4;
	int curIndex = 0;

	int rv = 0;
	char* exitKey = "exit";

	// curToken is the current "first" argument in remainder
	char* curToken = malloc(254);

	// keep operating on remainder until only one command left
	while(firstSpace != -1){

		// copy all of remainder into curToken, terminate it after first arg
		strncpy(curToken, remainder, (size_t) firstSpace);
		curToken[firstSpace] = '\0';

		tokenArr = realloc(tokenArr,
						indexCount * sizeof(*tokenArr));

		tokenArr[curIndex] = malloc(254);

		// unescape the current token and send it to the argument array
		char* unEsc = unescape(curToken, stderr);
		strcpy(tokenArr[curIndex], unEsc);
		free(unEsc);

		// remove first argument from remainder, rinse and repeat
		memmove(remainder, (remainder + firstSpace + 1), strlen(remainder));

		firstSpace = first_unquoted_space(remainder);
		indexCount++;
		curIndex++;
	}

	// we get here when our input line is down to last argument;
	// reallocate another index once more, fill it with the arg
	tokenArr = realloc(tokenArr, indexCount * sizeof(*tokenArr));
	tokenArr[curIndex] = malloc(254);

	char* unEsc = unescape(remainder, stderr);
	strcpy(tokenArr[curIndex], unEsc);
	free(unEsc);

	// add this thing to terminate our now-full argument array
	tokenArr[curIndex + 1] = (char *) 0;


	//check for an exit/invalid cmd; update return value  accordingly
	if(tokenArr[1] == (char *) 0 && strcmp(tokenArr[0], exitKey) == 0){
		printf("Exiting shell. Goodbye :)\n");
		rv = 2;
	}

	else{
		int execSuccess = execvp(tokenArr[0], tokenArr);
		if(execSuccess == -1){
			rv = 1;
		}
	}

	// filled tokenArr from index 0 to 1 past curIndex, so free those indices
	for(int i = 0; i <= curIndex + 1; i++){
		free(tokenArr[i]);
	}

	// clean up other dynamic vals and return
	free(curToken);
	free(remainder);
	free(tokenArr);

	return rv;
}
