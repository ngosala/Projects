#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include<setjmp.h>
#include <signal.h>

# define MAX_LENGTH 128 // Buffer size for arrays or any other input

// Declaration Section
char *PROMPT;
char *PATHVAR;
char *HOMEVAR;
static char* args[10];

jmp_buf (env);

// Set Home directory
void SetHomeDir(char *HomeDir){
    int changedir;
    changedir=chdir(HomeDir);
  setenv("HOME",HomeDir,1);
    if(changedir!=0){
        printf("\nError Setting Home Directory\n");

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
    // Reading HOMEVAR value
       while(1){
    pathstr=strstr(temp,"HOME");
    break;
    }
   
    HOMEVAR=strtok(pathstr,"\n");
  
    HOMEVAR=strstr(HOMEVAR,"/");

     // Reading Path value
    while(1){
    pathstr=strstr(temp,"PATH");
    break;
    }
    PATHVAR=strtok(pathstr,";.;");
   
     PATHVAR=strstr(PATHVAR,"/");
     
      setenv("PATH",PATHVAR,1);
 
    
    SetHomeDir(HOMEVAR);
    
 

}

//Function to create New Process
int createProcess(){

    int PID,FLAG=0;
    PID=fork();
    
    return PID;
}


// Signal Handler

void HandleSignal(int i){

    char response;
    signal(SIGINT, SIG_IGN);
    while(response!='Y' && response!='N' && response!='y' && response!='n'){
    
        printf("\n Are you Sure You want to EXIT ? (Y/N)");
        fflush(stdin);
        response=getchar();
        if(response=='Y' || response=='y'){
        
            printf("\nExiting Shell\n");
			exit(0);
        
        }
        else if (response=='N' || response=='n'){
        signal(SIGINT, HandleSignal);
        longjmp(env, 1);
        
        }
        else{
        
            printf("\nPlease provide valid response");
        
        }
      
        
    }

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
	int numOfCommands=1;
	char buffer[MAX_LENGTH];
	char temp[MAX_LENGTH];
	char* cmd2[MAX_LENGTH];
	int i,j,k;
     
        
        //find number of pipe commands
	cmd2[0] = strtok(readline, "$");
	while((cmd2[numOfCommands]=strtok(NULL, "$")) != NULL) 
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
        pipein = popen(cmd2[numOfCommands-1], "r");
        if(pipein==NULL){
        
            printf("Pipe Failed");
             longjmp(env, 1);
            
        }
        
	i=0;
	pipeout[i] = popen(cmd2[numOfCommands-2], "w");
          if(pipeout[i]==NULL){
        
             printf("Pipe Failed");
             longjmp(env, 1);   
        }

	while (fgets(buffer, MAX_LENGTH, pipein)) {
	
		fputs(buffer, pipeout[i]);
	} 
        //close pipes
	pclose(pipein);
        pclose(pipeout[i]);
	
	for(i=numOfCommands-2;i>0;i--) { 
       
        pipeout[i] = popen(cmd2[i], "r");
        
            if(pipeout[i]==NULL){
        
             printf("Pipe Failed");
             longjmp(env, 1);   
        }
        
	pipeout[i-1] = popen(cmd2[i-1], "w");
        
            if(pipeout[i+1]==NULL){
        
             printf("Pipe Failed");
             longjmp(env, 1);   
        }
        
        while (fgets(buffer, MAX_LENGTH, pipeout[i])) {
	
		fputs(buffer, pipeout[i-1]);
	}
    
	pclose(pipeout[i]);
	}


}


void Executecommand(char *command){


    if(fork()==0){
       
        
               args[0]="\0";    
   
        int i=execvp(command,args);
        
         if(i<0){
        
            printf("Error Processing Command or invalid Command");
            exit(1);
        
        }
        else
        {
             wait(NULL);
        }

        }
    
    longjmp(env,1);

    
}

void ExecutecommandWithArgs(char *command,char *argv[]){
        FILE *pipein;
	FILE *pipeout;
	int numOfCommands=0;
	char buffer[MAX_LENGTH];
	char* cmd1;
	char* cmd2;
	int i,j,k;
	
         
        pipein = popen(command, "r");
        if(pipein==NULL){
        
            printf("Error in Command");
            // longjmp(env, 1);
            
        }
	while (fgets(buffer, MAX_LENGTH, pipein)) {
	
		printf("%s",buffer);
	} 
        //close pipes
        
	pclose(pipein);
        longjmp(env,1);
        
}
void executecdCommand(char *argv[]){
    
    int changedir;
    changedir=chdir(argv[1]);
    if(changedir!=0){
         printf("Error Changing Directory or Invalid Directory\n");
    }
    
    longjmp(env,1);
    
}
void executeSetEnvCommand(char *readline){

    char *var1;
    char *constant1;
    var1=strtok(readline,"=");
    constant1=strtok(NULL,"=");
   int i= setenv(var1,constant1,1);
   if(i!=0){
       printf("Error Setting Environment Variable");
   }
    longjmp(env,1);


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
    char *result;
    if((strstr(readline,"="))!=NULL){
    
      var3=strtok(readline,"=");
      temp=strtok(NULL,"=");
      printf("%s",var3);
        
    }
    else{
        var3=NULL;
        temp=readline;
    
    }
    if(strstr(temp,"+")!=NULL){
        var4=(strtok(temp,"+"));
        var5=(strtok(NULL,"+"));
        if(strstr(var4,"$")){
            var4++;
        }
        if(strstr(var5,"$")){
            var5++;
        }
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
          if(strstr(var4,"$")){
            var4++;
        }
        if(strstr(var5,"$")){
            var5++;
        }
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
          if(strstr(var4,"$")){
            var4++;
        }
        if(strstr(var5,"$")){
            var5++;
        }
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
          if(strstr(var4,"$")){
            var4++;
        }
        if(strstr(var5,"$")){
            var5++;
        }
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
          if(strstr(var4,"$")){
            var4++;
        }
        if(strstr(var5,"$")){
            var5++;
        }
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
 
    longjmp(env,1);
    

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
    if(buffer!=NULL){
    printf("\n%s\n",buffer);
    }
    else{
    
        printf("Cannot read value");
    }
    longjmp(env,1);
}
void executeechoCommandForVar(char *command){

    char *token1;
    char *token2;
    char *buffer;
    char *temp;
    int i;
    token2=command;
    token2++;
    buffer=getenv(token2);
       if(buffer!=NULL){
    printf("\n%s\n",buffer);
    }
    else{
    
        printf("Cannot read value");
    }
    longjmp(env,1);
}
int main(int argc, char *argv[], char *envp[])
{
    char readline[MAX_LENGTH];
    char temp[MAX_LENGTH];
    char *buff1;
    char *buff2;
    PROMPT=NULL;
    

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
        fflush(stdin);
        setjmp(env);
        signal(SIGINT,HandleSignal);
        
        
       while(1){
           sleep(1);
           memset(readline, 0, sizeof(readline));
         PROMPT=getcwd(NULL, 0);
           strcat(PROMPT,">");
              printf("\n%s",PROMPT);
              fflush(stdin);
              gets(readline);
            //  fgets(readline,MAX_LENGTH,stdin);
                         if ((strstr(readline, "=>") != NULL) && (strstr(readline, "$") != NULL)){
                  
                              char *tmp1;
                              char *tmp2;
                              char *tmp3;
                              tmp1=strtok(readline,"=");
                              tmp2=strtok(NULL,"=");

                              tmp3=strcat(tmp1,tmp2);
                              memcpy(readline,tmp3,sizeof(tmp3));

              
                           }

			if (strcmp(readline, "exit") == 0) {
                            
				printf("\n GoodBye...\n");
				exit(0);
			}
			 else if(strstr(readline,"clear")!=NULL){
                        
                            Executecommand(readline);
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
                          else if(strstr(readline,"echo")!=NULL){
                        
                            executeechoCommand(readline);

                        }
                   
                        else if (strstr(readline, "$") != NULL || strstr(readline,"calc")!=NULL){
                            int i;
                            int count=0;
                            strcpy(temp,readline);
                            buff1=strtok(temp,"$");
                            while((strtok(NULL,"$"))){
                                count++;
                            }
                            
                            if(strstr(readline,"calc")!=NULL){
                                   buff2=strtok(readline," ");
                                   buff2=strtok(NULL," ");
                         
                                
                                executeCalcCommand(buff2);

                            }
                            else if(strstr(readline, "$") && (count==0)){
                             executeechoCommandForVar(temp);
                            }
                            else{
                            //ExecuteDollar(readline);
                                 executePipecommand(readline);
                            }
                            
                        }
                 
				
			else if (!strncmp(readline,"cd",2))
                          {
                            parser(readline, argv);
                            executecdCommand(argv);
                         }
                        else if(strstr(readline,"=")!=NULL){
                        
                            executeSetEnvCommand(readline);
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
       
	 
	 
}


    



