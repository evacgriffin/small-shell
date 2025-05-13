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

        // Skip blank lines and comment lines
        if(currCommand == NULL) {
            continue;
        }

        // Exit shell
        if(!strcmp(currCommand->argv[0], "exit")) {
            exitShell();
            break;
        }

        // Handle built-in commands
        if(!strcmp(currCommand->argv[0], "status")) {
            // Handle status
            continue;
        } else if(!strcmp(currCommand->argv[0], "cd")) {
            // Handle cd
            continue;
        }

        // Execute other commands
        executeCommand(currCommand);
	}

	return EXIT_SUCCESS;
}