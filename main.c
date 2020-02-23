#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "utils.h"

int execLine(char* lineIn);

int main (int argc, char* argv[]) {

	if(argc == 1){
		int childExit = 0;
		do{
			pid_t pid = fork();

			if(pid == 0){
				printf("> ");

				char* line = NULL;
				size_t len = 0;

				// take in entire input line, get rid of \n
				getline(&line, &len, stdin);
				line[strlen(line) - 1] = '\0';

				// parse and execute the input line
				childExit = execLine(line);

				exit(childExit);
			}

			else{
				int stat;
				wait(&stat);
				int exit = WEXITSTATUS(stat);
				//printf("Exit stat: "); printf("%d", exit); printf("\n");
				childExit = exit;
				if(exit == 1){
					printf("Command failed. Please try again.\n");
				}
			}

		}while(childExit != 2);

		exit(0);
	}

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

			if(pid == 0){
				int ret = execLine(line);
				if(ret == 1){
					fclose(input);
				}
				exit(ret);
			}

			else{
				wait(&stat);
				int exitStat = WEXITSTATUS(stat);

				if(exitStat == 1){
					printf("Invalid command read. Terminating execution\n");
					//break;
					free(line);
					fclose(input);
					exit(1);
					getlineSuccess = -1;
				}

				else{
					getlineSuccess = getline(&line, &sz, input);
					line[strlen(line) - 1] = '\0';
				}

			}

		}

		free(line);
		fclose(input);
		exit(0);

	}

	else{
		fprintf(stderr, "Error: shell must be run with 0 or 1 args.");
		exit(1);
	}

	return 0;

}


int execLine(char* lineIn){
	char** tokenArr = malloc(4 * sizeof(*tokenArr));

	char* remainder = lineIn;
	int firstSpace = first_unquoted_space(lineIn);
	int count = 4;
	int curIndex = 0;
	int rv = 0;
	char* exitKey = "exit";

	char* curToken = malloc(254);

	while(firstSpace != -1){
		strncpy(curToken, remainder, (size_t) firstSpace);
		curToken[firstSpace] = '\0';

		tokenArr = realloc(tokenArr,
						count * sizeof(*tokenArr));

		tokenArr[curIndex] = malloc(254);

		char* unEsc = unescape(curToken, stderr);
		strcpy(tokenArr[curIndex], unEsc);
		free(unEsc);

		memmove(remainder, (remainder + firstSpace + 1), strlen(remainder));

		firstSpace = first_unquoted_space(remainder);
		count++;
		curIndex++;
	}


	tokenArr = realloc(tokenArr, count * sizeof(*tokenArr));
	tokenArr[curIndex] = malloc(254);
	strcpy(tokenArr[curIndex], remainder);

	tokenArr[curIndex + 1] = (char *) 0;

	if(tokenArr[1] == (char *) 0 && strcmp(tokenArr[0], exitKey) == 0){
		printf("Exiting shell. Goodbye :)\n");
		rv = 2;
		count = 2;
	}

	else{
		int execSuccess = execvp(tokenArr[0], tokenArr);
		if(execSuccess == -1){
			rv = 1;
		}
	}

	for(int i = 0; i <= curIndex + 1; i++){
		free(tokenArr[i]);
	}

	free(curToken);
	free(remainder);
	free(tokenArr);

	return rv;
}
