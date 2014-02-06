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
 
PROMPT=strstr(PROMPT,"N");
    

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
    
    return FLAG;
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
        
        }
        else{
        
            printf("Please provide valid response");
        
        }
        
    }while(response!='Y' && response!='N' && response!='y' && response!='n');

}

int main(int argc, char *argv[], char *envp[])
{
    char readline[MAX_LENGTH];
    
       //  char str[]="nIkHeLeL";
  //  printf("%s",Lower(str));
    printf("\nWelcome to Custom Shell\n\n");
	printf("Nikhileswar Gosala\n");
	printf("Geetha Sankineni\n");
        printf("Harshitha Bandlam\n");
    
        InitialiseEnvironment();
        
        
        
       
        setjmp(getinput);
        
       signal(SIGINT,HandleSignal);
        
       printf("%s", PROMPT);
        
       while(1){
       
           getchar();
       
       
       }
	        
	   
}


    



