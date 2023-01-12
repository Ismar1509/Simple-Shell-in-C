//SimpleShell in C language - designed by Ismar Aganovic
//For the purpose of assignment - commands ls, pwd, grep and cd are implemented
//Some custom made are also available - scrclr, info, exit
//----------------------------------------------------------------------------------------------

//Stdio.h is included to use commands for input and output
#include <stdio.h>
//Stdlib.h is used for freeing space in the memory after usage - look at the note after main function
#include <stdlib.h>
//Stdbool.h is included to use Macros of this library for boolean expressions
#include <stdbool.h>
//String.h is included so we can use strtok to tokenize input of the user
#include <string.h>
//Sys/wait.h is included so we can use macros for waiting/holding the process and checking status
#include <sys/wait.h>
//Unistd.h is included so we can use execvp, perror, fork
#include <unistd.h>
//Errno.h is included so we can check if errors occur during shell run
#include <errno.h>

//Firstly, I created struct for built in commands that are going to be used
//We need to pass name of the command with the function that it is connected to
struct commandStorage {
    char *cmd_name; //Pointer for the name of command (string basically) e.g. "cd"
    void (*cmd_func)(char **args);
    //*function is pointer (string) to a function name e.g. changeDir
    //char **args is pointer to char pointer that is passed as an argument of the given function
};

//Implentation of directory change - cd - function
void changeDir(char **args) {
    //Firstly, we will check if the second argument (path) is empty, if it is empty, we change current directory to root
    //getenv("HOME") is used for getting root folder
    //tilde is not used as an valid argument to go to root, unlike original cygwin
    char* root = getenv("HOME");
    if (args[1] == NULL) {
        chdir(root);
    }else {
        //If the directory change with chdir was not successful, we will display error
        if (chdir(args[1]) != 0) {
            printf("Directory change error\n");
        }
    }
}

//Implementation of shell exit method
// char **args will be used for command "exit" to recognize method
void exitShell(char **args){
    //exit(0) is used to successfully exit the program/shell
    exit(0);
}

//Simple method to clear the screen - "clear" can be used also
void clearShell(char **args){ 
    system("clear");
}

//Function below is used just to display some basic information about the shell
//I put it just for details, even though it is not required and "help" can be used instead
//but the custom commands would not be shown except cd
void infoShell(char **args){
    char *shellInfo = "Custom commands:\n"
                    "cd     Lets you change your directory\n"
                    "NOTE: cd without path goes to root folder\n"
                    "exit   Lets you exit the shell\n"
                    "info   Lets you see these information\n"
                    "clrscr Lets you clear the screen\n"
                    "\n"
                    "Some of inherited commands from terminal:\n"
                    "pwd, ls, grep, who, ps...\n";
    printf("%s", shellInfo);
}

//Struct/Array with commands associated with functions from below
//All of them are made in the upper part of this code
struct commandStorage commands[] = {
    {"cd", changeDir},
    {"exit", exitShell},
    {"clrscr", clearShell},
    {"info", infoShell},
};


//In order to execute commands, we need to tokenize input
//We need to know input's length and we need to allocate some memory for it
//I used 32 for inputSize but I think that 16 would also be okay to use
//User input will be used as a parameter
char** tokenizer(char *input) {
    
    int storageSize = 0;
    int inputSize = 32;

    //We need to allocate memory for tokens storage
    //Since sizeof(char *) on 64-bit is 8, we would multiply it with 32 just to make sure to have enough memory
    char **tokensList = malloc(inputSize * sizeof(char*));

    //We need to check if there are errors, and exit as a failure if so
    //exit(1) is exit with failure
    if (!tokensList) {
        perror("Memory allocation error\n");
        exit(1);
    }

    //Initialization of delimiters - whitespace and enter/new row
    char *delimiters = " \n";

    //We perform tokenization on our input
    char *token = strtok(input, delimiters);

    //Until null pointer, we will keep adding tokens to our storage, starting with index of inputLength=0
    //New token implies incrementation by 1
    while (token != NULL) {
        tokensList[storageSize] = token;
        storageSize++;

        token = strtok(NULL, delimiters);
    }

    //Last element of our storage will be null pointer and we return our tokens
    tokensList[storageSize] = NULL;
    return tokensList;
}


//Execution of the commands happens here
void commandExe(char **args) {
    //We need to loop through our custom made commands from the struct above
    //If the first argument of user input is equal to the name of the command in the struct
    //We will passu user input to the corresponding function
    int i=0;
    while(i<4){
        if (strcmp(args[0], commands[i].cmd_name) == 0) {
            commands[i].cmd_func(args);
            return;
        }
        i++;
    }

    //We fork, creating new clone
    //Note: As I understood int also can be used, but after reading docs, I realised that pid_t is more reliable
    pid_t cPid = fork();

    //If child pid is 0 we will perform certain commands
    if (cPid == 0) {

        //To execute ls and commands related to it, we will firstly compare if first argument is equal to ls
        //I used execve, since ls is cygwin's system command and we can find it in bin, args are arguments that are passed
        //NULL is used since I did not use envp, and it worked fine
        if (strcmp(args[0], "ls") == 0)
            execve("/bin/ls", args, NULL);

        //To execute pwd, we will firstly compare if first argument is equal to pwd
        //I used execve, since pwd is cygwin's system command and we can find it in bin, args are arguments that are passed
        //NULL is used since I did not use envp, and it worked fine
        else if(strcmp(args[0], "pwd") == 0)
            execve("/bin/pwd", args, NULL);

        //To execute grep and commands related to it, we will firstly compare if first argument is equal to grep
        //I used execvp, since grep is cygwin's system command and we can find it in bin, but there are few problems with arguments
        //I was not 100% sure how to use it with grep, thus I used execvp, that does not need path
        // args are arguments that are passed, args[0] is command and args will be tokens from tokenStorage
        else if(strcmp(args[0], "grep") == 0){
            execvp(args[0], args);}

        //To execute those commands that are in struct, I used execvp, since path is not available because they are not
        //Cygwin's programms
        //Uses similar way of execution like grep
        //If error occurs we will exit as a failure
        else{
            execvp(args[0], args);
            printf("Command not found\n");
            exit(1);}

        //We wait for termination of process
    } else if (cPid > 0) {
        int status;
        waitpid(cPid, &status, 0);

        //Fork failed if it is less than 0
    } else {
        printf("Fork failed");
    }
}


//Main part where we print mysh# as a prompt after enter by user
//We allocate memory for user input
int main(int argc, char **argv, char **envp) {
    printf("WELCOME TO SIMPLE SHELL DESIGNED BY ISMAR\n"
            "------------------------------------------\n"
            "To see custom commands enter 'info'\n ");
    while (true) {
        printf("mysh# ");
        char *usrInput = NULL;
        size_t usrInputSize = 32;
        size_t output;
    
        usrInput = (char*)malloc(usrInputSize * sizeof(char));

        if(usrInput == NULL){
            printf("Memory allocation error\n");
            exit(1);
        }

        //Getline is used to get user input where, address of input and input size is passed
        output = getline(&usrInput, &usrInputSize, stdin);
        
        //We store tokens after tokenizing is performed for input
        char **tokens = tokenizer(usrInput);
       
       //If the first token is not empty/null, we proceed with execution
        if(tokens[0] != NULL){
            commandExe(tokens);
        }

        //NOTE: I did not know if I should use free()
        //I had some problems with memory (stackdumps), and found out that sometimes free is used to free space/memory
        //free(tokens);
        //free(usrInput);
    }
}