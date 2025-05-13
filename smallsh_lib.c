// Contains definitions for the function prototypes
#include "smallsh.h"

/*
 * Function: parseInput
 *      This function gets a new command line input, parses it, and returns
 * 		it as a pointer to a new commandLine structure.
 * 
 *      This code is based on:
 *      Title: sample_parser.c - Sample parser program provided on the Programming Assignment page
 *      URL: https://canvas.oregonstate.edu/courses/1999732/assignments/9997827?module_item_id=25329392
 *      Retrieved on: 05/13/2025
 * 
 *      returns: pointer to new commandLine structure
 */
struct commandLine *parseInput() {
	char input[INPUT_LENGTH];
	struct commandLine *currCommand = (struct commandLine *) calloc(1, sizeof(struct commandLine));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// Tokenize the input
	char *token = strtok(input, " \n");
	while(token) {
		if(!strcmp(token,"<")){
			currCommand->inputFile = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			currCommand->outputFile = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			currCommand->isBackground = true;
		} else{
			currCommand->argv[currCommand->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}

	return currCommand;
}