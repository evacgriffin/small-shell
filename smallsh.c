// Author:          Eva Griffin
// Course:          CS 372 - Operating Systems I
// Last Modified:   05/19/2025
// Assignment:      Programming Assignment 4 - SMALLSH

#include "smallsh.h"

void (*back_handler)(int);
bool foregroundMode = false;

/*
 * Signal Handler: SIGTSTP_enterForegroundMode
 *      This function is a signal handler for SIGTSTP. Runs when SIGTSTP is caught and switches
 *      the process to foreground only mode.
 * 
 *      This code is based on:
 *      Course Page: Exploration: Signal Handling API
 *      Title: Example: Custom Handlers for SIGINT, SIGUSR2, and Ignoring SIGTERM, etc.
 *      URL: https://canvas.oregonstate.edu/courses/1999732/pages/exploration-signal-handling-api?module_item_id=25329389
 *      Retrieved on: 05/19/2025
 * 
 *      Title: CS374 - Operating Systems 1 - Processes
 *      Author: Guillermo Tonsmann Ph.D.
 *      Published: June 2024 as a pdf document in the course materials
 *      Retrieved on: 05/19/2025
 * 
 *      receives: signal number as an integer
 *      returns: none
 */
void SIGTSTP_enterForegroundMode(int signo) {
    char *message = "\nEntering foreground-only mode (& is now ignored)\n";
    write(STDOUT_FILENO, message, 50);
    foregroundMode = true;

    // Swap handler for SIGTSTP: The exit foreground mode handler is now catching SIGTSTP
    back_handler = signal(SIGTSTP, SIGTSTP_exitForegroundMode);
    if(back_handler == SIG_ERR) {
        char *errorMsg = "Could not initialize handler for SIGTSTP\n";
        write(STDOUT_FILENO, errorMsg, 41);
        exit(1);
    }

    return;
}

/*
 * Signal Handler: SIGTSTP_exitForegroundMode
 *      This function is a signal handler for SIGTSTP. Runs when SIGTSTP is caught and switches
 *      the process out of foreground only mode.
 * 
 *      This code is based on:
 *      Course Page: Exploration: Signal Handling API
 *      Title: Example: Custom Handlers for SIGINT, SIGUSR2, and Ignoring SIGTERM, etc.
 *      URL: https://canvas.oregonstate.edu/courses/1999732/pages/exploration-signal-handling-api?module_item_id=25329389
 *      Retrieved on: 05/19/2025
 * 
 *      Title: CS374 - Operating Systems 1 - Processes
 *      Author: Guillermo Tonsmann Ph.D.
 *      Published: June 2024 as a pdf document in the course materials
 *      Retrieved on: 05/19/2025
 * 
 *      receives: signal number as an integer
 *      returns: none
 */
void SIGTSTP_exitForegroundMode(int signo) {
    char *message = "\nExiting foreground-only mode\n";
    write(STDOUT_FILENO, message, 30);
    foregroundMode = false;

    // Swap handler for SIGTSTP: The enter foreground mode handler is now catching SIGTSTP
    back_handler = signal(SIGTSTP, SIGTSTP_enterForegroundMode);
    if(back_handler == SIG_ERR) {
        char *errorMsg = "Could not initialize handler for SIGTSTP\n";
        write(STDOUT_FILENO, errorMsg, 41);
        exit(1);
    }

    return;
}

/*
 * SMALL SHELL
 *      Implementation of a small shell with the following functionality:
 *      - Command Prompt
 *      - Ignores comments and blank lines
 *      - Has special built-in commands exit, cd, and status
 *      - Executes other standard shell commands
 *      - Redirects I/O
 *      - Executes commands in foreground and background
 *      - Handles signals SIGINT and SIGTSTP
 * 
 *      Signal code is based on:
 *      Course Page: Exploration: Signal Handling API
 *      Title: Example: Custom Handlers for SIGINT, SIGUSR2, and Ignoring SIGTERM, etc.
 *      URL: https://canvas.oregonstate.edu/courses/1999732/pages/exploration-signal-handling-api?module_item_id=25329389
 *      Retrieved on: 05/19/2025
 */
int main() {
    struct commandLine *currCommand = NULL;
    int exitStatus = 0;

    // Parent ignores SIGINT signal
    struct sigaction SIGINT_ignore = {0};
    SIGINT_ignore.sa_handler = SIG_IGN;
    sigaction(SIGINT, &SIGINT_ignore, NULL);

    // Parent starts out by entering foreground mode when first catching SIGTSTP
    back_handler = signal(SIGTSTP, SIGTSTP_enterForegroundMode);
    if(back_handler == SIG_ERR) {
        printf("Could not initialize handler for SIGTSTP\n");
        exit(1);
    }

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
        currCommand = parseInput(foregroundMode);

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