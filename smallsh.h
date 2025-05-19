// Header file for smallsh.c
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

// Represents one command line entry
struct commandLine
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *inputFile;
	char *outputFile;
	bool isBackground;
};

// Function prototypes
void SIGTSTP_enterForegroundMode(int signo);
void SIGTSTP_exitForegroundMode(int signo);
struct commandLine *parseInput(bool foregroundMode);
void changeWorkingDirectory(struct commandLine *currCommand);
int executeCommand(struct commandLine *currCommand);