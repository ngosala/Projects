
#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#define ARRAY_LENGTH 256


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



Schema *
testSchema ( int numAttr, char *names[],  DataType dt[],int sizes[],int keys[])
{
  Schema *result;
  int i;
  char **cpNames = (char **) malloc(sizeof(char*) * numAttr);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * numAttr);
  int *cpSizes = (int *) malloc(sizeof(int) * numAttr);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < numAttr; i++)
    {
      cpNames[i] = (char *) malloc(2);
      strcpy(cpNames[i], names[i]);
    }
  memcpy(cpDt, dt, sizeof(DataType) * numAttr);
  memcpy(cpSizes, sizes, sizeof(int) * numAttr);
  memcpy(cpKeys, keys, sizeof(int));

  result = createSchema(numAttr, cpNames, cpDt, cpSizes, 1, cpKeys);

  return result;
}

char* Upper(char *temp){
     
     
     int i;
     for(i=0;temp[i]!='\0';i++){
     
       if ((temp[i]>=97) && (temp[i]<=122)){
         temp[i]=temp[i]-32;
         }
      }
     
     return temp;
     
     }



/*
 * AUTHOR: Nikhil  
 * DESC: Parse the sql stmts and execute the respective methods
 */
int main(int argc, char *argv[]){

    char readline[ARRAY_LENGTH];
    char temparray[ARRAY_LENGTH];
    char *temp;
    char *temp1;
    char *tableName;
    char *length;
    int charLength;
    int numattr=0,k,count=0,numTuples=0;
    Schema *schema;
    RID *rids;
    char columns[ARRAY_LENGTH][ARRAY_LENGTH];
    
   
    int i=0,j=0;
    
        if(fork() == 0) {
			execv("/usr/bin/clear", argv);
			exit(1);
		}
	else wait(NULL);
        
        printf("\n SQL parser\n");
        
        while(1){
        printf("\n >>");
        
        fflush(stdin);
        gets(readline);
        
        memcpy(temparray,readline,sizeof(readline));
        
        if(strstr(readline,"create table") || strstr(readline,"CREATE TABLE")){
        
            parser(readline,argv);
            
            tableName=strtok(argv[2],"(");
            
            temp=strstr(temparray,"(");
            
            temp++;
            
            temp1=strtok(temp,",");
            while(temp1!=NULL){
                strcpy(columns[i],temp1);
            temp1=strtok(NULL,",");
            i++;
            }
            j=i;
            numattr=j;
   strcpy(columns[i-1],strtok(columns[i-1],")"));

    char* colname[numattr];
    char* datatype[numattr];
    DataType dt[numattr];
    int sizes[numattr];
     int keyattr[numattr];
            for(i=0;i<j;i++){
          
       
                parser(columns[i],argv);
                
                colname[i]=argv[0];
                datatype[i]=argv[1];
                if(strstr(Upper(datatype[i]),"VARCHAR")) {
                     dt[i]=DT_STRING;
                    length=strtok(datatype[i],"(");
                    length=strtok(NULL,"(");
                    charLength=strtok(length,")");
                    if(length==NULL){
                    
                        printf("\nWrong Format");
                                 break;
                    }
                        sizes[i]=atoi(charLength);
                }
                        if(strstr(Upper(datatype[i]),"INT")){
                        dt[i]=DT_INT;
                        }
                         else if(strstr(Upper(datatype[i]),"FLOAT")){
                            dt[i]=DT_FLOAT;
                        }
                    else if(strstr(Upper(datatype[i]),"BOOL")){
                            dt[i]=DT_BOOL;                        }
                
                
                else{
                 sizes[i]=0;
                }
                
                if(strstr(Upper(datatype[i]),"PRIMARYKEY")){
                    
                keyattr[i]=1;
                    
                }
                else{
                 keyattr[i]=0;
                }
               
                
            }
     
     for(k=0;k<sizeof(keyattr)/sizeof(keyattr[0]);k++){
         if(keyattr[i]==1){
             count++;
          
          }
   
     }
     if(count>0){
         
         printf("wrong format...cannot contain more than one primary key");
         break;
     
     }
 
     schema=testSchema(numattr,colname,dt,sizes,keyattr);
     initRecordManager(NULL);
     createTable(Upper(tableName),schema);
     printf("Table Created Successfully");
     shutdownRecordManager();

        }
        
        if(strstr(Upper(readline),"INSERT INTO")){
        
              parser(readline,argv);
            
            tableName=strtok(argv[2],"(");
            
            temp=strtok(temparray,"(");
            temp=strtok(NULL,"(");
            strcpy(columns[0],temp);
       strcpy(columns[0],strtok(columns[0],")"));
       
        char* values[i];
        i=0;
       values[0]=strtok(columns[0],",");
       while(values[i]!=NULL){
           i++;
          values[i]=strtok(NULL,",");
       }
       
             numTuples=i;
             rids = (RID *) malloc(sizeof(RID) * numTuples);
             RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
             initRecordManager(NULL);
        int fd = open(Upper(tableName), O_WRONLY);    
        if(fd<0){
            printf("Database Doesn't Exist");
            break;
        }

                openTable(table, tableName);
                schema=table->schema;
                 Record *result;
                 Value *value;
                createRecord(&result, schema);
                for(i = 0; i < numTuples; i++)
                {
                  if(schema->dataTypes[i]==DT_INT || schema->dataTypes[i]==DT_FLOAT || schema->dataTypes[i]==DT_BOOL){
                  MAKE_VALUE(value, schema->dataTypes[i], atoi(values[i]));
                  }
                  if(schema->dataTypes[i]==DT_STRING){
                  MAKE_STRING_VALUE(value, values[i]);
                  }
               
                   setAttr(result, schema, i, value);
                   freeVal(value);
                 
                }
                  insertRecord(table,result); 
                  rids[i] = result->id;
printf("Inserted Successfully");
               closeTable(table);
               shutdownRecordManager();
              
             
                  }
        
         if(strstr(Upper(readline),"UPDATE TABLE")){
             
           parser(readline,argv);
            
            tableName=strtok(argv[2],"(");
            
            temp1=strtok(temparray,"(");
            temp=strtok(NULL,"(");
            strcpy(columns[0],temp);
       strcpy(columns[0],strtok(columns[0],")"));
       
       temp1=strtok(temp1,"=");
       temp1=strtok(NULL,"=");
       
        char* values[i];
        i=0;
       values[0]=strtok(columns[0],",");
       while(values[i]!=NULL){
           i++;
          values[i]=strtok(NULL,",");
       }
       
             numTuples=i;
             rids = (RID *) malloc(sizeof(RID) * numTuples);
             RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
             initRecordManager(NULL);
        int fd = open(Upper(tableName), O_WRONLY);    
        if(fd<0){
            printf("Database Doesn't Exist");
            break;
        }
       
                openTable(table, Upper(tableName));
                schema=table->schema;
                               
                  Record *result;
                  Value *value;
                  createRecord(&result, schema);
                for(i = 0; i < numTuples; i++)

                {
                  if(schema->dataTypes[i]==DT_INT || schema->dataTypes[i]==DT_FLOAT || schema->dataTypes[i]==DT_BOOL){
                  MAKE_VALUE(value, schema->dataTypes[i], atoi(values[i]));
                  }
                  if(schema->dataTypes[i]==DT_STRING){
                  MAKE_STRING_VALUE(value, values[i]);
                  }
               
                   setAttr(result, schema, i, value);
                   freeVal(value);
            
                   
                }
                       result->id=rids[i] ;
                   updateRecord(table,result);
printf("Updated Successfully");
               closeTable(table);
               shutdownRecordManager();
               
         }
        
         if(strstr(Upper(readline),"DELETE FROM")){
             
           parser(readline,argv);
            
            tableName=strtok(argv[2],"(");
            
            temp=strtok(temparray,"(");
            temp=strtok(NULL,"(");
            strcpy(columns[0],temp);
       strcpy(columns[0],strtok(columns[0],")"));
       
       int r_id=atoi(columns[0]);
             numTuples=i;
             rids = (RID *) malloc(sizeof(RID) * numTuples);
RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
                           initRecordManager(NULL);
        int fd = open(Upper(tableName), O_WRONLY);    
        if(fd<0){
            printf("Database Doesn't Exist");
            break;
        }

                openTable(table, Upper(tableName));
                schema=table->schema;
               deleteRecord(table,rids[r_id]);
printf("Deleted Successfully");
               closeTable(table);
               shutdownRecordManager();
         }
              
        
        }
        }
