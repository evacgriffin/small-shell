// Author:          Eva Griffin
// Course:          CS 372 - Operating Systems I
// Last Modified:   05/13/2025
// Assignment:      Programming Assignment 4 - SMALLSH
// References:
//
// Title:          
// URL:            
// Retrieved On:   


#include "smallsh.h"

int main() {
    struct commandLine *currCommand;

	while(true)
	{
		currCommand = parseInput();
		for(int i = 0; i < sizeof(*currCommand->argv); i++) {
			if(currCommand->argv[i] != NULL) {
				printf("Current command: %s\n", currCommand->argv[i]);
			}
		}
	}

	return EXIT_SUCCESS;
}