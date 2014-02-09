#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include<setjmp.h>
#include <signal.h>
#include "ErrorCodes.h"

# define MAX_LENGTH 1024 // Buffer size for arrays or any other input



// Declaration Section
char *PROMPT;
char *PATH;
char *HOME;
static char* args[10];

jmp_buf (getinput);

// Set Home directory

EC SetHomeDir(char *HomeDir){
int errno=0;
    int changedir;
    changedir=chdir(HomeDir);
    if(changedir!=0){
        printf("Error Setting Home Directory :%s\n",strerror(errno));
        
        return EC_OK;
    }
    else{
    
       return EC_HOMEDIRERROR;
    }
}

// Initialise Environment

EC InitialiseEnvironment(){
    
    char temp[MAX_LENGTH];
    char *pathstr;
    int i=0;
    char *str;
    FILE *fptr=fopen("PROFILE.txt","rb");
    

    
    while(!(feof(fptr))){
    
        
        
        fgets(temp,MAX_LENGTH,fptr);
        
        
    
    }
    fclose(fptr);
    // Reading HOME value
       while(1){
    pathstr=strstr(temp,"HOME");
    break;
    }
   
    HOME=strtok(pathstr,"\n");
  
    HOME=strstr(HOME,"/");

     // Reading Path value
    while(1){
    pathstr=strstr(temp,"PATH");
    break;
    }
    PATH=strtok(pathstr,";.;");
   
     PATH=strstr(PATH,"/");
     

    
   // Reading Prompt value
     while(1){
    pathstr=strstr(temp,"PROMPT");
    break;
    }
     PROMPT=strtok(pathstr,";.;");
 
     PROMPT=strstr(PROMPT,"My");
    

    SetHomeDir(HOME);
    
    return EC_OK;

}

// function to convert Uppser case to lower case

char* Lower(char *temp){
    
    
    int i;
    for(i=0;temp[i]!='\0';i++){
    
    
    if ((temp[i]>=65) && (temp[i]<=90)){
        temp[i]=temp[i]+32;
         }
    
  
    
     }
    
    return temp;
    
    }

// function to convert lower case to upper
char* Upper(char *temp){
    
    
    int i;
    for(i=0;temp[i]!='\0';i++){
    
      if ((temp[i]>=97) && (temp[i]<=122)){
        temp[i]=temp[i]-32;
        }
     }
    
    return temp;
    
    }




//Function to create New Process
int createProcess(){

    int PID,FLAG=0;
    PID=fork();
    
    return PID;
}


// Signal Handler

void HandleSignal(int c){

    char response;
    do{
    
        printf("\n Are you Sure You want to EXIT ? (Y/N)");
        fflush(stdin);
        response=getchar();
        if(response=='Y' || response=='y'){
        
            printf("\nExiting Shell\n");
			exit(0);
        
        }
        else if (response=='N' || response=='n'){
        signal(SIGINT, HandleSignal);
        longjmp(getinput, 1);
           // printf("Continue");
           // printf("%s",PROMPT);
        
        }
        else{
        
            printf("Please provide valid response");
        
        }
       // response==NULL;
        
    }while(response!='Y' && response!='N' && response!='y' && response!='n');

}

/* Function to parse the input command */

void parser(char *readline, char**argv) {
    
	while (*readline != '\0') {
		while (*readline == ' ' || *readline == '\t' || *readline == '\n')
			*readline++ = '\0';
		*argv++ = readline;
		while (*readline != '\0' && *readline != ' ' && *readline != '\t' && *readline != '\n')
			readline++;
	}
	*argv = '\0';
}

void Redirect(char *cmd,char *argv[]){
        char *command;
	char line[100];
	FILE *fpipe;
	FILE *outfile;
	char *filename;
        int i;
        for(i=0;i<sizeof(argv)-1;i++){
            printf("\nag %s",argv[i]);
            if(argv[i]==">"){
                printf("here %s",argv[i]);
                filename=argv[i++];
                break;

            }

        }
     
        command=strtok(cmd,">");
     //   printf("command %s",filename);

	// Open Pipe to redirect the command
	if( ! (fpipe =(FILE *)popen(command, "r") ) )
	{
		printf("Unable to execute the command !\n");
		exit(1);
	}

	// Open File Stream to write
	if(!(outfile=fopen((char *)filename, "wb"))){
		printf("Open file failed!\n");
		exit(1);
	}

	/* read stream from input file and write the stream to the output file */
	while(fgets(line,sizeof(line),fpipe))
		fprintf(outfile,"%s", line);

	pclose(fpipe);
	fclose(outfile);
	free(command);


}

void piping(char *pipecommand){
    
                int fd[2];
		pid_t pid;
                char* token1;
                char* token2[MAX_LENGTH];
                int count=0;
                token1 = strtok(pipecommand, "|");
                while((token2[count]=strtok(NULL, "|")) != NULL) {
             
                    count++;
                }
                
                
                int i=0,j=0;
                while (token2[i] == ' ') {
		j = 0;
		while (token2[j]) {
			token2[j] = token2[j + 1];
			j++;
		}
		i++;
	}
                
                
                
                
                
                
		/* create the pipe */
		if (pipe(fd) == -1) {
			printf("Pipe failed");
		}
		/* fork a child process */
		pid = fork();
		if (pid < 0) { 
			printf("Fork Failed");
			return 1;
		}
               printf("%s",token2[1]);
                


}

void ExecuteCommand(char *command){
FILE *pipein_fp;
FILE* pipeout_fp[256];

    if(fork()==0){
        
        if(strstr(command,"=>")!=NULL){
        
            Redirect(command,NULL);
        
        }
        else{
        
               args[0]="\0";    
   
        int i=execvp(command,args);
        
         if(i<0){
        
            printf("Some Error");
            exit(1);
        
        }
        else
        {
             wait(NULL);
        }

        }
    


    }
}
void executeCdCommand(char *argv[]){
    
   
    int changedir;
    char* curwd[MAX_LENGTH];
    getcwd(curwd,MAX_LENGTH);
    changedir=chdir(curwd);
    if(changedir!=0){
        printf("couldnt change dir1");
    }
   
    
  
    changedir=chdir(argv[1]);
    if(changedir!=0){
        printf("couldnt change dir2");
    }
    
    
    
}
void executePipeCommand(char *readline){


}
int main(int argc, char *argv[], char *envp[])
{
    char readline[MAX_LENGTH];
    char temp[MAX_LENGTH];
    
       //  char str[]="nIkHeLeL";
  //  printf("%s",Lower(str));
   
        
        //clear the window
	if(fork() == 0) {
			execv("/usr/bin/clear", argv);
			exit(1);
		}
	else wait(NULL);
        
            printf("\nWelcome to Custom Shell\n\n");
	printf("Nikhileswar Gosala\n");
	printf("Geetha Sankineni\n");
        printf("Harshitha Bandlam\n");
    
        InitialiseEnvironment();
        setjmp(getinput);
        
       signal(SIGINT,HandleSignal);
        
     //  printf("%s", PROMPT);
        
       while(1){
         //  sleep(1);
     printf("%s",PROMPT); 
              gets(readline);
                 
                 // puts(readline);
                        
                  fflush(stdin);
                  
			//parser(readline, argv);

			if (strcmp(argv[0], "exit") == 0) {
                            
				printf("\n GoodBye...\n");
				exit(0);
			}
                       
                        
			else 			
			if (strstr(readline, ">") != NULL) {
                            strcpy(temp,readline);
                            parser(readline, argv);
                            Redirect(temp,argv);
			} else if (strcmp(readline, "getpid") == 0){
				int pid=createProcess();
                        printf("%d",pid);
                        }
			else if (strstr(readline, "|") != NULL)
				executePipeCommand(readline);
			else if (strstr(readline,"cd")!=NULL)
                          {
                            parser(readline, argv);
                            executeCdCommand(argv);
                         }
                        
                        else
                            ExecuteCommand(readline);

                        
       
                        	
                        
                    
       
       }
	 
	 
}


    



