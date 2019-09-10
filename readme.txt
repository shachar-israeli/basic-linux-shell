
israelish
203713094
Shachar Israeli


This software is a terminal (shell/bash) simulator with linux
command

support (without folder control) and "&" , "|" , "<" , ">" , ">>"
and "2>" argumenet support.

There can be up to 2 processes running per client (one for command, one for shell/bash CLI).
there is a set limit of an input up to 512 characters and a limit of hostname display of 512 characters

how to install:
open linux terminal, navigate to the folder containing ex2.c
using the "cd" command (confirm it by using ls command)
incase you have makefile, type make and the program will
automaticily be compiled, if you don't, type gcc -Wall ex2.c -o ex2
and your program will automaticily be compiled

to activate:
open linux terminal, navigate to ex1 executeable file
location using "cd" command (confirm it using ls command) and type
./ex2

to operate:
once you are inside ex2 type any command you want

*incase a command execution was successfull, return code will be
updated

*incase the command "done" was sent, the terminal simulator will
exit

*incase of empty command input(space enter, enter...) or any
failure to fetch input from user,
last return code will be retained and a new line will be made for
the simulator command

*the program has 3 static variables - cmd length, num of cmd, flag_background (for &)

------------------ functions -------------------------------
* incase of memory allocation failure, program will exit with
message "Memory allocation failed"



#define#:


#define MAX_LENGTH 512
#define END_PROGRAM "done\n"
#define CHANGE_DIR "cd"

#define FILE_PUT_IN ">"
#define FILE_PUT_APPEND ">>"
#define FILE_OUT_ERR "2>"
#define FILE_IN "<"
#define PIPE_SIGN "|"

#define BACKROUND_SIGN "&"


#functions:


void sig_chld(int signo);
void print();
char** makeArgv(int countWords ,char input[]);
int numOfWords(char inputCopy[]);
void checkIfSpace(char input[]);
void done();
void fatherProcess(char** argv,char ** left, char ** right);
void childProcess (char** argv, int countWords ,char ** left , char ** right);
int check_sign(char** argv,int countWords,char* sign);

void makePipe (char ** left, char ** right);
char** rightArr(char** argv,int countWords,int divide);
char** leftArr(char** argv,int countWords,int divide);
void free_arr (char ** arr , int length);
void makeRedirection (char** argv,char ** left, char ** right);
int isRedirection ( char * pInput);






------------------ functions -------------------------------

void print();
input: none
output: prints the command line (the directory).


char** makeArgv(int countWords ,char input[]);
input: int - the number of words, char[] - user input
output: a dynamic memory allocated string array of the input
string divided into words, last element is NULL


int numOfWords(char inputCopy[]);
input: char[] - user input
output: integer describing how many words (words meaning paragraphs
divided by ' ' and '\n') there are in a string


void childProcess (char** argv);
input: char** - the array of words (argv) 
output: executes the command


void fatherProcess(char** argv, int* cmdLength, int*
input: char** - the array of words (argv), int* - num of cmd (by address) , int* - cmd length (by address)
output: executes the command


void checkIfSpace(char input[], int *numOfCmd);
input: char[] - user input, int* - num of cmd (by address)
output: checks if the command is actually 'space/s'


void done(int numOfCmd, int cmdLength);
input: int - num of cmd, int - cmd length
output: prints cmd length and num of cmd




int main() command:
input: standart main input
output: the shell CLI simulatorProgram has been fully tested with valgrind to test for memory
leak, no leak was found.
