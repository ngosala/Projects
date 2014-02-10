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
char* PROMPT[MAX_LENGTH];
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
   //  PROMPT=strtok(pathstr,";.;");
 
    // PROMPT=strstr(PROMPT,"My");
    
     getcwd(PROMPT,MAX_LENGTH);
     strcat(PROMPT,">");

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
        /*
        for(i=0;i<sizeof(argv)-1;i++){
            printf("\nag %s",argv[i]);
            if(argv[i]==">"){
                printf("here %s",argv[i]);
                filename=argv[i++];
                break;

            }

        }
     */
        command=strtok(cmd,">");
        filename=strtok(NULL,">");
      // printf("command%s123",filename);

	// Open Pipe to redirect the command
	if( ! (fpipe =(FILE *)popen(command, "r") ) )
	{
		printf("Unable to execute the command !\n");
		exit(1);
	}

	// Open File Stream to write
	if(!(outfile=fopen(filename, "wb"))){
		printf("Open file failed!\n");
                //perror(filename);
		exit(1);
	}

	/* read stream from input file and write the stream to the output file */
	while(fgets(line,sizeof(line),fpipe))
		fprintf(outfile,"%s", line);

	pclose(fpipe);
	fclose(outfile);
	//free(command);


}

void executePipeCommand(char *readline){

        FILE *pipein;
	FILE* pipeout[MAX_LENGTH];
	int i, j, k, numOfCommands=0;
	char readbuf[MAX_LENGTH];
	char* cmd1;
	char* cmd2[MAX_LENGTH];
	
	
        //find number of pipe commands
	cmd1 = strtok(readline, "|");
	while((cmd2[numOfCommands]=strtok(NULL, "|")) != NULL) 
	numOfCommands++;
	
	for(k=0;k< numOfCommands; k++) { 
	i = 0;
	while (cmd2[k][i] == ' ') {
		j = 0;
		while (cmd2[k][j]) {
			cmd2[k][j] = cmd2[k][j + 1];
			j++;
		}
		i++;
	}
	}
        
        printf("%s",cmd2[1]);
	
	if ((pipein = popen(cmd1, "r")) == NULL) {
		printf("Error executing command\n");
		perror("popen");
	    longjmp(getinput, 1);
	}

	i=0;
	
	if ((pipeout[i] = popen(cmd2[i], "w")) == NULL) {
		printf("Error execting command\n");
		perror("popen");
		longjmp(getinput, 1);
		
	}
	

	while (fgets(readbuf, 80, pipein)) {
	
		fputs(readbuf, pipeout[i]);
	} 
	

	pclose(pipein);
        pclose(pipeout[i]);
	
	for(i=0;i<numOfCommands-1;i++) { 
    	if ((pipeout[i] = popen(cmd2[i], "r")) == NULL) {
		printf("Error executing command\n");
		perror("popen");
	    longjmp(getinput, 1);
	}
	
	if ((pipeout[i+1] = popen(cmd2[i+1], "w")) == NULL) {
		printf("Error execting command\n");
		perror("popen");
		longjmp(getinput, 1);
		
	}
	
    while (fgets(readbuf, MAX_LENGTH, pipeout[i])) {
	
		fputs(readbuf, pipeout[i+1]);
	}
    
	pclose(pipeout[i]);
	}


}


void ExecuteCommand(char *command){


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
void ExecuteCommandWithArgs(char *command,char *argv[]){
    pid_t pid;
    pid=fork();
    int k;
    char *cmd=argv[0];
  //  printf("%s",pid);
       for(k=0;k<sizeof(argv)-1;k++){
           argv[k]=argv[k+1];
        }
        
           
         char *args[64];
        char **next = args;
        
        char *temp = strtok(command," ");
        strcpy(cmd,temp);
        int i=0;
        while (temp != NULL)
        {
            temp = strtok(NULL, " ");
            //*next++ = temp;
           // printf("%s\n", temp);
            args[i]=temp;
            i++;
            
        }
        args[i] = NULL;
        
        for(k=0;k<sizeof(args)-1;k++){
            //printf("args %s",args[k]);
        }
        
    if(pid>0){
 
        int j= execvp(cmd, argv);
        
         if(j<0){
        
            printf("Some Error");
            exit(1);
        
        }
        else
        {
             wait(NULL);
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
           sleep(1);
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
			else if (strstr(readline, "|") != NULL){
                            executePipeCommand(readline);
                        }
				
			else if (strstr(readline,"cd")!=NULL)
                          {
                            parser(readline, argv);
                            executeCdCommand(argv);
                         }
                        
                        else{ 
                             strcpy(temp,readline);
                             parser(readline, argv);
                             // printf("argv %s",argv[0]);
                             if(argv[1]!=NULL){
                             
                                 ExecuteCommandWithArgs(temp,argv);
                                 
                             }
                             else
                            ExecuteCommand(readline);
                        }


       }
	 
	 
}


    



