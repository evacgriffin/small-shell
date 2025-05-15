// Author:          Eva Griffin
// Course:          CS 372 - Operating Systems I
// Last Modified:   05/15/2025
// Assignment:      Programming Assignment 4 - SMALLSH 


#include "smallsh.h"

int main() {
    struct commandLine *currCommand = NULL;
    int exitStatus = 0;

	while(true)
	{
        // Wait for background processes to finish
        int childPid;
        while((childPid = waitpid(-1, &exitStatus, WNOHANG)) > 0) {
            if(exitStatus != 0) {
                exitStatus = 1;
            }
            printf("background pid %d is done: exit value %d\n", childPid, exitStatus);
            fflush(stdout);
        }

        // New command prompt
        currCommand = parseInput();

        // Skip blank lines and comment lines
        if(currCommand == NULL) {
            continue;
        }

        // Exit the shell
        if(!strcmp(currCommand->argv[0], "exit")) {
            break;
        }

        // Handle built-in commands
        if(!strcmp(currCommand->argv[0], "status")) {
            printf("exit value %d\n", exitStatus);
            fflush(stdout);
            continue;
        } else if(!strcmp(currCommand->argv[0], "cd")) {
            changeWorkingDirectory(currCommand);
            continue;
        }

        // Execute other commands
        exitStatus = executeCommand(currCommand);
        if(exitStatus != 0) {
            exitStatus = 1;
        }
	}

	return EXIT_SUCCESS;
}