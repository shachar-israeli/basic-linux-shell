
/*
NAME: Shachar Israeli
ID: 203713094
DATE: 27/04/2018


*
* This is an extension for ex1 -> simulate shell
* supporting: SIGINT (ctrl+c) ,background (& sign), pipe, rediection
*/
 
 


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_LENGTH 512
#define END_PROGRAM "done\n"
#define CHANGE_DIR "cd"

#define FILE_PUT_IN ">"
#define FILE_PUT_APPEND ">>"
#define FILE_OUT_ERR "2>"
#define FILE_IN "<"
#define PIPE_SIGN "|"

#define BACKROUND_SIGN "&"

// functions:


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




//main : 

static int f_backround ;      // to check if there is & in the command
static int numOfCmd = 0;  // counters
static int cmdLength = 0;
static int left_size;       
static int right_size;

static  int pipe_place ;
static int redirection;


int main()
{
    char input[MAX_LENGTH];
 
	 
    do
    {
   	
        signal (SIGINT,sig_chld);
        signal (SIGCHLD,SIG_DFL);
        
        pipe_place = -1;           // pipe flag and the place
    	redirection = -1;          // redirection flag and place
        f_backround = 0 ;          // check if & is in the end
        
        char** left = NULL;    // for pipe|redirection
        left_size = 0;
        char** right = NULL ;
        right_size = 0;
        
        
		print(); //print path
        fgets(input, MAX_LENGTH, stdin);

        if (strcmp(input, END_PROGRAM) != 0)
        {
            char inputCopy[MAX_LENGTH];
            strcpy(inputCopy, input);

            int countWords = numOfWords(inputCopy);

            if(countWords != 0)    //there is a command in the input
            {
                char** argv = makeArgv(countWords,input);
				pipe_place  = check_sign(argv,countWords + 1,PIPE_SIGN);
				
				
				if (strcmp(argv[0],CHANGE_DIR) == 0)
  				  {
       			 	int ret;                // change directory
       			 	if (argv[1] != NULL)
					ret = chdir(argv[1]);   // for the CD Command. dont need to cound it as numOfCmd
					free_arr (argv , countWords + 1);
					continue;
    }

                pid_t x;
                x = fork();
                //childProcess(argv,countWords);
                if(x < 0) // fork(); didnt work. quit
                {
                    printf("ERR\n");
                    exit(0);
                }
                if (x == 0)
                {
                    childProcess(argv, countWords,left,right);
                }

                else    // in father process
                {
                    fatherProcess(argv,left,right);  // send the counters to the father proccess
                    free_arr (argv , countWords + 1);           
                }
            }
            else
                checkIfSpace(input);
        }
      
    }
    while (strcmp(input, END_PROGRAM) != 0);
    done();
    return 0;
}


char** makeArgv(int countWords ,char input[]) // create the main arr command
{
    char** argv = (char**)malloc((countWords + 1) * sizeof(char*)); // make the argv arr
    if (argv == NULL)
    {
        printf("ERR\n");
        exit(1);
    }

    int i = 0;
    char* pInput;
    pInput = strtok (input," \n");// just if there is " " in the input

    while (i < countWords)
    {
        argv[i] = (char*)malloc(strlen(pInput)+1 * sizeof(char));   // put the commands in the arr
        if(argv[i] == NULL)
        {
            printf("ERR \n");
            exit(1);
        }
        strcpy(argv[i], pInput);
        if (isRedirection (pInput) == 1)
        	redirection = i ;                 // save the place of redirection
        i++;
        pInput = strtok (NULL, " \n");
    }

    argv[i] = NULL;  // the last index
    return argv;
}

void print()
{
    char path[MAX_LENGTH];
    printf("%s@%s>", getpwuid(0)->pw_name,getcwd(path, MAX_LENGTH)); // if you want to get the real id put getpwuid(geteuid());
}

int numOfWords(char inputCopy[])   // count the num of words that we have in the user input
{

    int countWords = 0;
    char* pInput;

    pInput = strtok (inputCopy," \n");
    while (pInput != NULL)  // count the words   // xeyes &
    {
        if (strcmp(pInput,BACKROUND_SIGN) == 0 && strtok (NULL, " \n") == NULL)     
        {
            countWords--;   // dont count '&' as a word.
            f_backround = 1 ; // flag backround is on
        }
        countWords++;
        pInput = strtok (NULL, " \n");
    }
    return countWords;
}

void childProcess (char** argv, int countWords,char ** left, char ** right)      // in child
{
    signal (SIGINT,sig_chld);           // for ^c
	
	if ( redirection != -1 )        
	{
		
		left_size = redirection + 1;       // take the main arr and divide it to 2 arrays.
    	right_size = countWords - redirection; 
		left =  leftArr ( argv, countWords, redirection);      	
		right = rightArr( argv, countWords, redirection);
    	 
		makeRedirection (argv,left,right);       // do the redirection
    
	}
    if (pipe_place != -1 && redirection == -1)	//only pipe without redirection
    {
    	left_size = pipe_place + 1;               // take the main arr and divide it to 2 arrays.
    	right_size = countWords - pipe_place;
    	
    	left =  leftArr ( argv, countWords, pipe_place);
		right = rightArr( argv, countWords, pipe_place);
		
        makePipe (left,right);
	}
    if (redirection == -1 && pipe_place == -1) // no spicel commands. just do execvp
    {
    
    	if ( strcmp(argv[0],CHANGE_DIR) != 0)
    	{
        int checkExec;
        checkExec = execvp(argv[0],argv); 
        }   // executing the command
      //  if (checkExec == -1 && strcmp(argv[0],CHANGE_DIR) != 0)
          //  printf("%s: command not found\n", argv[0]);
       
    }
     exit(0);
}

void fatherProcess(char** argv,char ** left, char ** right)  // in father
{

	if (f_backround == 0 )     //if backround is on - dont quit the program
        wait(0);
        
        
	//if (strcmp(argv[0],CHANGE_DIR) == 0)
  //  {
   //     int ret;                // change directory
    //    ret = chdir(argv[1]);   // for the CD Command. dont need to cound it as numOfCmd
        
  //  }
  //  else
  //  {
        cmdLength += strlen(argv[0]); // to count the string's length of the cmd
        numOfCmd++;
   // }
    
    if (pipe_place != -1 || redirection != -1)  // if there is left|right arrays
    {
    	free_arr (left , left_size);
    	free_arr (right , right_size);
   	}
   	
   	 

}


void checkIfSpace(char input[])           // check in its space or Enter
{
    if (strcmp(input, "\n") != 0)  //the input is spaces
    {
        //printf(": command not found\n");
        numOfCmd++;      //we count spaces ,but don't count enters
    }
}

void done()    // finish the Ex
{
    printf("Num of cmd: %d\nCmd length: %d\nBye !\n",numOfCmd,cmdLength);
    exit(0);
}


void sig_chld(int signo)
{
    if( signo == SIGINT)
        signal (SIGCHLD, sig_chld);

    if( signo == SIGCHLD)
    {
       // pid_t   pid;
       // int  stat;
      //  pid = wait(&stat);
		int status ;
		waitpid(-1, &status, WNOHANG);
    }
}
int check_sign(char** argv,int countWords, char* sign)       // return the index of the sign.
{
    for ( int i = 0; i < countWords-1 ; i ++)
        if (strcmp(argv[i], sign) == 0 && i != 0 )
            return i;  // the place of the pipe left | right
    return -1;	//if no need pipe
}


char** leftArr(char** argv,int countWords,int divide)    
{

    char** left = (char**)malloc((divide + 1) * sizeof(char*)); // make the left arr
    if (left == NULL)
    {
        printf("ERR\n");
        exit(1);
    }

    int i = 0 ;                          //make left
    while (i < divide)
    {
        left[i] = (char*)malloc(strlen (argv[i]) + 1 * sizeof(char));   // put the commands in the arr
        if(left[i] == NULL)
        {
            printf("ERR \n");
            exit(1);
        }
        strcpy(left[i], argv[i]);
        i++;
    }
    left[i] = NULL;  // the last index

    return left;

}


char** rightArr(char** argv,int countWords,int divide)
{

    char** right = (char**)malloc((countWords - divide) * sizeof(char*)); // make the right arr
    if (argv == NULL)
    {
        printf("ERR\n");
        exit(1);
    }

    int j = 0;
    int i = divide + 1;
    while (j < countWords-divide - 1 )    
    {
        right[j] = (char*)malloc(strlen(argv[i])+ 1 * sizeof(char));   // put the commands in the arr
        if(right[j] == NULL)
        {
            printf("ERR \n");
            exit(1);
        }
        strcpy(right[j], argv[i]);
        j++;
        i++;
    }
    right[j] = NULL;  // the last index

    return right;
}

void makePipe (char ** left, char ** right)
{

    pid_t child;

    int pipe_fd[2];        // pipe create

    if ((pipe(pipe_fd)) == -1)
    {
        perror("ERR");
        exit(0);
    }

    child = fork();           //now we have 2 childs. 1 from the main and new 1 now

    if(child < 0)
    {
        perror("ERR");
        exit(0);
    }


    if(child == 0)              // this child
    {
        close(pipe_fd[0]);
        int value = dup2(pipe_fd[1],STDOUT_FILENO);
        close(pipe_fd[1]);
        if(value == -1)
        {

        }
        int checkExec;
        checkExec = execvp(left[0],left);    // executing the command
        if (checkExec == -1 )
        {
            printf("ERR");
            exit(1);
        }
        exit(0);
    }

    else
    {                            // the other kid
        wait(0);
        close(pipe_fd[1]);
        int value = dup2(pipe_fd[0],STDIN_FILENO);
        close(pipe_fd[0]);

        if(value == -1)
        {
            printf("ERR");
            exit(1);
        }

        int checkExec;
        checkExec = execvp(right[0],right);    // executing the command
        if (checkExec == -1 )
        {
            printf("ERR");
            exit(1);
        }
    }
}


void free_arr (char ** arr , int length )
{

    for (int i = 0; i < length ; i++)   
        free(arr[i]);
    free(arr);


}

void makeRedirection (char** argv,char ** left, char ** right)      // redirction
{

    int fd = -1; 

    if ( strcmp(argv[redirection],FILE_PUT_APPEND) == 0 )       // >> donr teplace the text 
    {
        fd = open(right[0], O_CREAT | O_APPEND | O_WRONLY, S_IRWXU | S_IRWXG |S_IRWXO);
         if (fd < 0) 
         {
            perror("Err - open input file");
            exit(0);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    else if ( strcmp(argv[redirection],FILE_PUT_IN) == 0 )   //  > replace the text in the file 
    {
        fd = open(right[0], O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG |S_IRWXO);
        if (fd < 0) 
         {
            perror("Err - open input file");
            exit(0);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);


    }
    else if ( strcmp(argv[redirection],FILE_OUT_ERR) == 0 )       // 2> ERROR commands
    {
        fd = open(right[0], O_WRONLY|O_CREAT|O_APPEND, S_IRWXU);
        if (fd < 0) 
         {
            perror("Err - open input file");
            exit(0);
        }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    else              // <
    {
        fd = open(right[0], O_RDONLY , 0);
        if (fd < 0) 
         {
            perror("Err - open input file");
            exit(0);
        }
        dup2(fd, STDIN_FILENO);
        close ( fd ) ;

    }

    if( pipe_place != -1)              // if there is a pipe in the left side of the arr.
    	{
    	
    	
    	char ** temp_left =  leftArr ( left, redirection, pipe_place);     // we gonna replace the left and right arr
    	char ** temp_right = rightArr( left, redirection,pipe_place); 
    	
    	free_arr (left , left_size);      // free the redirction arrays
    	free_arr (right , right_size);
    	
    	left = temp_left;
    	right = temp_right;
    	
    	left_size = pipe_place + 1;
    	right_size = redirection - pipe_place;
    	
    	
        makePipe (left,right);      
        }
        
        
    else          // no pipe needed - > just open\get the file
    {
        int checkExec;
        checkExec = execvp(left[0],left);    // executing the command
		//if (checkExec == -1 && strcmp(left[0],CHANGE_DIR) != 0)
         //   printf("%s: command not found\n", left[0]);
   }

    wait(0);
}


int isRedirection ( char * pInput)      // bool function

{

	if ( strcmp(pInput,FILE_PUT_APPEND) == 0 )
		return 1;

	if ( strcmp(pInput,FILE_PUT_IN) == 0 )
		return 1;
	
	if ( strcmp(pInput,FILE_OUT_ERR) == 0 )
		return 1;

	if ( strcmp(pInput, FILE_IN) == 0 )
		return 1;		
		
	return 0 ;
}












