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
 *      returns: pointer to new commandLine structure or NULL to skip the line
 */
struct commandLine *parseInput() {
	char input[INPUT_LENGTH];
	struct commandLine *currCommand = (struct commandLine *) calloc(1, sizeof(struct commandLine));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

    // Ignore blank lines
    if(!strcmp(input,"\n")) {
        return NULL;
    }

	// Tokenize the input
	char *token = strtok(input, " \n");

    // Ignore lines starting with '#'
    if(token[0] == '#') {
        return NULL;
    }

	while(token) {
		if(!strcmp(token,"<")) {
			currCommand->inputFile = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")) {
			currCommand->outputFile = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")) {
            char *nextToken = strtok(NULL," \n");
            if(nextToken) {
                currCommand->argv[currCommand->argc++] = strdup(token);
                token = strdup(nextToken);
                continue;
            } else {
                // Only run command in background if & is the last word
                currCommand->isBackground = true;
            }
		} else {
			currCommand->argv[currCommand->argc++] = strdup(token);
		}
		token = strtok(NULL," \n");
	}

	return currCommand;
}

/*
 * Function: changeWorkingDirectory
 *      This function handles the cd command and chances the current working directory.
 * 
 *      receives: pointer to current command structure
 *      returns: none
 */
void changeWorkingDirectory(struct commandLine *currCommand) {
    char *homeDirectoryVarName = "HOME";
    if(currCommand->argc == 1) {
        // No arguments: Change directory to HOME
        chdir(getenv(homeDirectoryVarName));
    } else {
        // Get the current working directory
        char currWorkingDirectory[INPUT_LENGTH];
        size_t directoryNameSize = sizeof(currWorkingDirectory);
        getcwd(currWorkingDirectory, directoryNameSize);

        // Check if path is absolute or relative
        if(!strncmp(currWorkingDirectory, currCommand->argv[1], strlen(currWorkingDirectory))) {
            // Change working directory to specified absolute path
            chdir(currCommand->argv[1]);
        } else {
            // Path is relative
            // If specified path does not begin with a /, append a / to the curr working dir
            if((currCommand->argv[1])[0] != '/') {
                int length = strlen(currWorkingDirectory);
                currWorkingDirectory[length] = '/';
                currWorkingDirectory[length + 1] = '\0';
            }
            // Concatenate curr working dir and specified relative path
            strcat(currWorkingDirectory, currCommand->argv[1]);
            // Change working directory to this new path
            chdir(currWorkingDirectory);
        }
    }

    return;
}

/*
 * Function: executeCommand
 *      This function executes a non-built-in command by forking a child process that runs the command.
 * 
 *      This code is based on:
 *      Course Page: Exploration: Process API - Executing a New Program
 *      Title: Example - Using exec() with fork()
 *      URL: https://canvas.oregonstate.edu/courses/1999732/pages/exploration-process-api-executing-a-new-program?module_item_id=25329382
 *      Retrieved on: 05/13/2025
 * 
 *      receives: pointer to current command structure
 *      returns: none
 */
void executeCommand(struct commandLine *currCommand) {
    int childStatus;
    // Fork a new child process
    pid_t spawnPid = fork();

    switch(spawnPid) {
    case -1:
        perror("fork() failed\n");
        exit(1);
        break;
    case 0:
        // Child process branch
        execvp(currCommand->argv[0], currCommand->argv);
        perror("execvp error");
        exit(1);
        break;
    default:
        // Parent process branch, parent waits for child to finish
        spawnPid = waitpid(spawnPid, &childStatus, 0);
        break;
    }
}