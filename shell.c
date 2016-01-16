#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_LINE 80
#define MAX_HISTORY 10

char ** args;
int should_run = 1;
int tokenNumber, letterNumber;
char currentChar;
char buffer[MAX_LINE/2 + 1];
int background = 0, isAmpersand = 0;
int hasMoreCharacters = 1;

int numberOfTokensInCommand[MAX_HISTORY];
int runInBackground[MAX_HISTORY];
char ** previousCommands[MAX_HISTORY];
int numInHistory = 0;
int minHistory = 0;
int maxHistory = 0;

void displayHistory();
void addCommandToHistory(char * argList[], int inBackground, int numTokens);
char ** copyArgList(char * argList[], int numTokens);
void makeSpaceInHistory();
char * makeCommandOneLine(int commandIndex); 
int getCommandIndex(int commandNumber);
void parseHistoryMarker(char * argList[]);
int handleCommand(int isInBackground); 
void exitIfCommanded();
void resetArguments();
void handleParentProcess(int inBackground);
void handleChildProcess();
void handleCommandRepeat();
void handleEndOfToken();
void checkAndHandleEndOfLine();
void checkAndHandleHistoryCommands();
void handleCharacter();
void startNewLine();

int main(){
	while(should_run){
		startNewLine();
		while(hasMoreCharacters){
			currentChar = getc(stdin);
			handleCharacter();
			checkAndHandleEndOfLine();
		}

		checkAndHandleHistoryCommands();

		addCommandToHistory(args, background, tokenNumber);
		int returnCode = handleCommand(background);

		if(returnCode != -1){
			return returnCode;
		}
	
		exitIfCommanded();
		resetArguments(tokenNumber);
	}

	return 0;
}

void displayHistory(){
	int i;
	char * commandInOneLine;
	for(i = numInHistory - 1; i >= 0; i--){
		commandInOneLine = makeCommandOneLine(i);
		printf("%d %s\n", minHistory + i,commandInOneLine);
		free(commandInOneLine);
	}
}

char * makeCommandOneLine(int commandIndex){
	char * commandLine = (char *)calloc(MAX_LINE + 1, sizeof(char));
	char ** argList = previousCommands[commandIndex];
	int i;
	strcpy(commandLine, argList[0]);
	for(i = 1; i < numberOfTokensInCommand[commandIndex]; i++){
		strcat(commandLine," ");
		strcat(commandLine, argList[i]);
	}
	
	if(runInBackground[commandIndex]){
		strcat(commandLine," &");	
	}
	strcat(commandLine, "\0");

	return commandLine;	
}

void makeSpaceInHistory(){
	int i;
	for(i = 0; i < MAX_HISTORY - 1; i++){
		numberOfTokensInCommand[i] = numberOfTokensInCommand[i+1];
		previousCommands[i] = previousCommands[i+1];
		runInBackground[i] = runInBackground[i+1];
	}
	minHistory++;
}

char ** copyArgList(char * argList[], int numTokens){
	int i;
	char ** copiedArgList = (char **)calloc(numTokens, sizeof(char *));
	for(i = 0; i < numTokens; i++){
		copiedArgList[i] = argList[i];
	}
	return copiedArgList;
}

void addCommandToHistory(char * argList[], int inBackground, int numTokens){
	if(strcmp(argList[0], "")){
		if(numInHistory >= MAX_HISTORY){
			makeSpaceInHistory();
			numInHistory--;
		}
		if(0 == minHistory){
			minHistory++;
		}
		maxHistory++;
		previousCommands[numInHistory] = copyArgList(argList, numTokens);
		numberOfTokensInCommand[numInHistory] = numTokens;
		runInBackground[numInHistory] = inBackground;
		
		numInHistory++;
	}
}		

int getCommandIndex(int commandNumber){
	if(commandNumber >= minHistory && commandNumber <= maxHistory){
		return commandNumber - minHistory;
	}
	else{
		return -1;
	}		
}

void parseHistoryMarker(char * argList[]){
	int commandNumber = atoi(argList[0]+1);
	if(commandNumber){
		int commandIndex = getCommandIndex(commandNumber);
		if(-1 == commandIndex){
			printf("No such command in history.\n");
		}
		else{	
			tokenNumber = numberOfTokensInCommand[commandIndex];
			args = copyArgList(previousCommands[commandIndex], tokenNumber);
			background = runInBackground[commandIndex];
			printf("%s\n", makeCommandOneLine(commandIndex));
		}
	}
}

int handleCommand(int inBackground){
		pid_t pid;
		pid = fork();

		if(pid < 0){
			fprintf(stderr, "Fork failed.");
			return 1;
		}
		else if (pid == 0){
			handleChildProcess();
			return 0;
		}
		else{
			handleParentProcess(inBackground);
		}

		return -1;
}

void exitIfCommanded(){
	if(strcmp(args[0], "exit") == 0){
			should_run = 0;
	}
}

void resetArguments(){
	int k;
	for(k = 0; k < tokenNumber; k++){
		args[k] = 0;
	} 
}

void handleParentProcess(int inBackground){
	if(!inBackground){
		wait(NULL);
	}
	else{
		signal(SIGCHLD, SIG_IGN);
		inBackground  = 0;
	}
}

void handleChildProcess(){
	if(strcmp(args[0], "history") == 0){
		displayHistory();
	}
	execvp(args[0], args); 
	fflush(stdout);
}

void handleCommandRepeat(){
	if(numInHistory == 0){
		printf("No commands in history.\n");
	}
	else{
		args = copyArgList(previousCommands[numInHistory - 1], numberOfTokensInCommand[numInHistory - 1]);
		background = runInBackground[numInHistory -1];
		tokenNumber = numberOfTokensInCommand[numInHistory - 1];
		printf("%s\n", makeCommandOneLine(numInHistory-1));
	}
}

void handleEndOfToken(){
	buffer[letterNumber] = '\0';
	if(strcmp(buffer, "&") == 0){
		background = 1;
		isAmpersand = 1;
	}
	if(!isAmpersand){		
		int bufferSize = strlen(buffer)+1;
		args[tokenNumber] = (char *)calloc(bufferSize, sizeof(char));
		strcpy(args[tokenNumber], buffer);
		tokenNumber++;
	}	
	else{
		isAmpersand = 0;
	}


	letterNumber =0;
}


void checkAndHandleEndOfLine(){
	if('\n' == currentChar){
		hasMoreCharacters = 0;
	}
}

void checkAndHandleHistoryCommands(){
	if(strcmp(args[0], "!!") == 0){	
		handleCommandRepeat();
	}
	else if (args[0][0] == '!'){
		parseHistoryMarker(args);
	}
}

void handleCharacter(){
	if(!isspace(currentChar)){
		buffer[letterNumber] = currentChar;
		letterNumber++; 
	}
	else{	
		handleEndOfToken();
	}
}

void startNewLine(){
	printf("osh> ");
	fflush(stdout);
	tokenNumber = 0;
	letterNumber = 0;
	args = (char**)calloc(MAX_LINE/2 + 1, sizeof(char *));
	hasMoreCharacters = 1;
}
