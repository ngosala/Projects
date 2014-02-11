#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include<setjmp.h>
#include <signal.h>

# define MAX_LENGTH 1024 // Buffer size for arrays or any other input



// Declaration Section

char *PATH;
char *HOME;
static char* args[10];

jmp_buf (getinput);

// Set Home directory

void SetHomeDir(char *HomeDir){
int errno=0;
    int changedir;
    changedir=chdir(HomeDir);
    if(changedir!=0){
        printf("\nError while Setting Home Directory :%s\n",strerror(errno));

    }
  
}

// Initialise Environment

void InitialiseEnvironment(){
    
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

    
  
    
    SetHomeDir(HOME);
    
 

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
	char buffer[MAX_LENGTH];
	FILE *fpipe;
	FILE *outfile;
	char *filename;
      
       
        command=strtok(cmd,"=>");
        filename=strtok(NULL,"=>");
      // printf("command%s123",filename);

	// Open Pipe to redirect the command
	if( ! (fpipe =(FILE *)popen(command, "r") ) )
	{
		printf("Unable to execute the command !\n");
		exit(1);
	}

	
	if((outfile=fopen(filename, "wb"))==NULL){
		printf("Open file failed!\n");
		exit(0);
	}

	
	while(fgets(buffer,sizeof(buffer),fpipe))
		fprintf(outfile,"%s", buffer);

	pclose(fpipe);
	fclose(outfile);
}

void executePipecommand(char *readline){

        FILE *pipein;
	FILE *pipeout[MAX_LENGTH];
	int numOfCommands=0;
	char buffer[MAX_LENGTH];
	char* cmd1;
	char* cmd2[MAX_LENGTH];
	int i,j,k;
	
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
        pipein = popen(cmd1, "r");
        if(pipein==NULL){
        
            printf("Pipe Failed");
             longjmp(getinput, 1);
            
        }
        
	i=0;
	pipeout[i] = popen(cmd2[i], "w");
          if(pipeout[i]==NULL){
        
             printf("Pipe Failed");
             longjmp(getinput, 1);   
        }

	while (fgets(buffer, MAX_LENGTH, pipein)) {
	
		fputs(buffer, pipeout[i]);
	} 
        //close pipes
	pclose(pipein);
        pclose(pipeout[i]);
	
	for(i=0;i<numOfCommands-1;i++) { 
       
        pipeout[i] = popen(cmd2[i], "r");
        
            if(pipeout[i]==NULL){
        
             printf("Pipe Failed");
             longjmp(getinput, 1);   
        }
        
	pipeout[i+1] = popen(cmd2[i+1], "w");
        
            if(pipeout[i+1]==NULL){
        
             printf("Pipe Failed");
             longjmp(getinput, 1);   
        }
        
        while (fgets(buffer, MAX_LENGTH, pipeout[i])) {
	
		fputs(buffer, pipeout[i+1]);
	}
    
	pclose(pipeout[i]);
	}


}


void Executecommand(char *command){


    if(fork()==0){
       
        
               args[0]="\0";    
   
        int i=execvp(command,args);
        
         if(i<0){
        
            printf("Error Processing Command");
            exit(1);
        
        }
        else
        {
             wait(NULL);
        }

        }
    


    
}

void ExecuteDollar(char *readline){

        FILE *pipein;
	FILE *pipeout;
	int numOfCommands=0;
	char buffer[MAX_LENGTH];
	char* cmd1;
	char* cmd2;
	int i,j,k;
	
        //find number of pipe commands
	cmd1 = strtok(readline, "$");
        cmd2 = strtok(NULL,"$");
        pipein = popen(cmd2, "r");
        if(pipein==NULL){
        
            printf("Pipe Failed");
             longjmp(getinput, 1);
            
        }
        
	i=0;
	pipeout = popen(cmd1, "w");
          if(pipeout==NULL){
        
             printf("Pipe Failed");
             longjmp(getinput, 1);   
        }

	while (fgets(buffer, MAX_LENGTH, pipein)) {
	
		fputs(buffer, pipeout);
	} 
        //close pipes
	pclose(pipein);
        pclose(pipeout);
         longjmp(getinput,1);
}
/*
void ExecutecommandWithArgs(char *command,char *argv[]){
    pid_t pid;
    pid=fork();
    int k;
    char *cmd=argv[0];
  
       for(k=0;k<sizeof(argv)-1;k++){
             
           argv[k]=argv[k+1];
           printf("ar %s",argv[k]);
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
        longjmp(getinput,1);
  
}
 */

void ExecutecommandWithArgs(char *command,char *argv[]){
    FILE *pipein;
	FILE *pipeout;
	int numOfCommands=0;
	char buffer[MAX_LENGTH];
	char* cmd1;
	char* cmd2;
	int i,j,k;
	
        //find number of pipe commands
	//cmd1 = argv[0];
   
        pipein = popen(command, "r");
        if(pipein==NULL){
        
            printf("Error in Command");
             longjmp(getinput, 1);
            
        }
	while (fgets(buffer, MAX_LENGTH, pipein)) {
	
		printf("%s",buffer);
	} 
        //close pipes
	pclose(pipein);
        longjmp(getinput,1);
  
}
void executecdCommand(char *argv[]){
    
   
    int changedir;

    printf("%s",argv[1]);
    changedir=chdir(argv[1]);
    if(changedir!=0){
        printf("couldnt change dir");
    }
    
    
    
}
void executeSetEnvCommand(char *readline){

    char *var1;
    char *constant1;
    var1=strtok(readline,"=");
    constant1=strtok(NULL,"=");
    setenv(var1,constant1,0);


}

void executeCalcCommand(char *readline){
    int var1;
    int var2;
    char *var5;
    char *var4;
    char *var3;
    char *temp;
    char *buff;
    int res;
    if((strstr(readline,"="))!=NULL){
    
      var3=strtok(readline,"=");
      temp=strtok(NULL,"=");
        
    }
    else{
    
        temp=readline;
    
    }
    if(strstr(temp,"+")!=NULL){
        var4=(strtok(temp,"+"));
        var5=(strtok(NULL,"+"));
        
        if((buff=getenv(var4))!=NULL){
        var1=atoi(buff);
        }
        else{
        var1=atoi(var4);
        }
         if((buff=getenv(var5))!=NULL){
        var2=atoi(buff);
        }
        else{
        var2=atoi(var5);
        }
       
        res=(var1+var2);
        printf("%d",res);
    
    }
    else if(strstr(temp,"-")!=NULL){
          var4=(strtok(temp,"-"));
        var5=(strtok(NULL,"-"));
        
        if((buff=getenv(var4))!=NULL){
        var1=atoi(buff);
        }
        else{
        var1=atoi(var4);
        }
         if((buff=getenv(var5))!=NULL){
        var2=atoi(buff);
        }
        else{
        var2=atoi(var5);
        }
       
        res=(var1-var2);
        printf("%d",res);
    }
    else if(strstr(temp,"*")!=NULL){
    
            var4=(strtok(temp,"*"));
        var5=(strtok(NULL,"*"));
        
        if((buff=getenv(var4))!=NULL){
        var1=atoi(buff);
        }
        else{
        var1=atoi(var4);
        }
         if((buff=getenv(var5))!=NULL){
        var2=atoi(buff);
        }
        else{
        var2=atoi(var5);
        }
       
        res=(var1*var2);
        printf("%d",res);
    }
    else if(strstr(temp,"/")!=NULL){
    
             var4=(strtok(temp,"/"));
        var5=(strtok(NULL,"/"));
        
        if((buff=getenv(var4))!=NULL){
        var1=atoi(buff);
        }
        else{
        var1=atoi(var4);
        }
         if((buff=getenv(var5))!=NULL){
        var2=atoi(buff);
        }
        else{
        var2=atoi(var5);
        }
       
        res=(var1/var2);
        printf("%d",res);
    }
    else if(strstr(temp,"%")!=NULL){
    
           var4=(strtok(temp,"%"));
        var5=(strtok(NULL,"%"));
        
        if((buff=getenv(var4))!=NULL){
        var1=atoi(buff);
        }
        else{
        var1=atoi(var4);
        }
         if((buff=getenv(var5))!=NULL){
        var2=atoi(buff);
        }
        else{
        var2=atoi(var5);
        }
       
        res=(var1%var2);
        printf("%d",res);
    }
   // if(var3!=NULL){
    
     //   setenv(var3,res,0);
    
   // }
    
    
    

}
void executeechoCommand(char *command){

    char *token1;
    char *token2;
    char *buffer;
    char *temp;
    int i;
    token1=strtok(command," ");
    token2=strtok(NULL," ");
    token2++;
    buffer=getenv(token2);
    printf("\n%s",buffer);
}
int main(int argc, char *argv[], char *envp[])
{
    char readline[MAX_LENGTH];
    char temp[MAX_LENGTH];
    char *PROMPT;

        //clear the window
	if(fork() == 0) {
			execv("/usr/bin/clear", argv);
			exit(1);
		}
	else wait(NULL);
        
        printf("\nWelcome to Custom Shell\n");
	printf("Nikhileswar Gosala\n");
	printf("Geetha Sankineni\n");
        printf("Harshitha Bandlamudi\n");
    
        InitialiseEnvironment();
        setjmp(getinput);
        
       signal(SIGINT,HandleSignal);
        
     //  printf("%s", PROMPT);
        
       while(1){
           sleep(1);
PROMPT=getcwd(NULL, 0);
  strcat(PROMPT,">");
           printf("\n%s",PROMPT); 
           
         
//printf("hello");
              gets(readline);
    
                 fflush(stdin);

			if (strcmp(argv[0], "exit") == 0) {
                            
				printf("\n GoodBye...\n");
				exit(0);
			}
                       
                        
			else 			
			if (strstr(readline, "=>") != NULL) {
                            strcpy(temp,readline);
                            parser(readline, argv);
                            Redirect(temp,argv);
			} else if (strcmp(readline, "getpid") == 0){
				int pid=createProcess();
                                printf("%d",pid);
                        }
			else if (strstr(readline, "|") != NULL){
                            executePipecommand(readline);
                        }
                          else if(strstr(readline,"echo")!=NULL){
                        
                            executeechoCommand(readline);

                        }
                        else if (strstr(readline, "$") != NULL){
                            ExecuteDollar(readline);
                        }
				
			else if (strstr(readline,"cd")!=NULL)
                          {
                            parser(readline, argv);
                            executecdCommand(argv);
                         }
                        else if(strstr(readline,"=")!=NULL){
                        
                            executeSetEnvCommand(readline);
                        }
                        else if((strstr(readline,"+") || strstr(readline,"-") ||strstr(readline,"*") ||strstr(readline,"/") ||strstr(readline,"%")!=NULL)){
                        
                           executeCalcCommand(readline);
                        }
                        else{ 
                             strcpy(temp,readline);
                             parser(readline, argv);
                             if(argv[1]!=NULL){
                             
                                 ExecutecommandWithArgs(temp,argv);
                                 
                             }
                             else
                            Executecommand(readline);
                        }


       }
       free(argv);
	 
	 
}


    



