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
 *      receives: boolean indicating which mode the shell is in
 *      returns: pointer to new commandLine structure or NULL to skip the line
 */
struct commandLine *parseInput(bool foregroundMode) {
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

    // Foreground mode enabled
    if(foregroundMode) {
        currCommand->isBackground = false;
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
 *      Course Page: Exploration: Processes and I/O
 *      Title: Example: Redirecting both Stdin and Stdout
 *      URL: https://canvas.oregonstate.edu/courses/1999732/pages/exploration-processes-and-i-slash-o?module_item_id=25329390
 *      Retrieved on: 05/13/2025
 * 
 *      Course Page: Exploration: Signal Handling API
 *      Title: Example: Custom Handlers for SIGINT, SIGUSR2, and Ignoring SIGTERM, etc.
 *      URL: https://canvas.oregonstate.edu/courses/1999732/pages/exploration-signal-handling-api?module_item_id=25329389
 *      Retrieved on: 05/19/2025
 * 
 *      receives: pointer to current command structure
 *      returns: exit status or terminating signal as an integer
 */
int executeCommand(struct commandLine *currCommand) {
    int exitStatus = 0;

    // Fork a new child process
    pid_t spawnPid = fork();

    switch(spawnPid) {
    case -1:
        perror("fork() failed\n");
        exit(1);
        break;
    case 0:
        // Child process branch

        // All child processes ignore SIGTSTP
        struct sigaction SIGTSTP_ignore = {0};
        SIGTSTP_ignore.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &SIGTSTP_ignore, NULL);

        // SIGINT signal processing
        if(currCommand->isBackground) {
            // Background child ignores SIGINT
            struct sigaction SIGINT_ignore = {0};
            SIGINT_ignore.sa_handler = SIG_IGN;
            sigaction(SIGINT, &SIGINT_ignore, NULL);
        } else {
            // Foreground child handles SIGINT with default action
            struct sigaction SIGINT_default = {0};
            SIGINT_default.sa_handler = SIG_DFL;
            sigaction(SIGINT, &SIGINT_default, NULL);
        }

        // Open input file
        int result;
        if(currCommand->inputFile) {
            int inputFileDescriptor = open(currCommand->inputFile, O_RDONLY);
            if(inputFileDescriptor == -1) {
                printf("cannot open %s for input\n", currCommand->inputFile);
                fflush(stdout);
                exit(1);
            }
            // Redirect stdin to the input file
            result = dup2(inputFileDescriptor, 0);
            if(result == -1) {
                perror("input file dup2()");
                exit(1);
            }
        }

        // Open output file
        if(currCommand->outputFile) {
            int outputFileDescriptor = open(currCommand->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(outputFileDescriptor == -1) {
                perror("output file open()");
                exit(1);
            }
            // Redirect stdout to the output file
            result = dup2(outputFileDescriptor, 1);
            if(result == -1) {
                perror("output file dup2()");
                exit(1);
            }
        }

        // Confirm background process
        if(currCommand->isBackground) {
            printf("background pid is %d\n", getpid());
            fflush(stdout);

            // Redirect I/O for background process if no files specified by the user
            if(!(currCommand->inputFile)) {
                int bgInput = open("/dev/null", O_RDONLY);
                if(bgInput == -1) {
                    perror("bg input open()");
                    exit(1);
                }
                result = dup2(bgInput, 0);
                if(result == -1) {
                    perror("bg input dup2()");
                    exit(1);
                }
            }

            if(!(currCommand->outputFile)) {
                int bgOutput = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(bgOutput == -1) {
                    perror("bg output open()");
                    exit(1);
                }
                result = dup2(bgOutput, 1);
                if(result == -1) {
                    perror("bg output dup2()");
                    exit(1);
                }
            }
        }

        // Execute command
        execvp(currCommand->argv[0], currCommand->argv);
        printf("%s: no such file or directory\n", currCommand->argv[0]);
        fflush(stdout);
        exit(1);
        break;
    default:
        // Parent process branch

        // Parent does not wait for a background process
        if(!(currCommand->isBackground)) {
            // Parent waits for child to finish
            spawnPid = waitpid(spawnPid, &exitStatus, 0);
        }
        break;
    }

    return exitStatus;
}