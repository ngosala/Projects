#include <stdio.h>
#include<stdlib.h>
struct buffer{
    char *data;
    struct buffer *Next;

}*start=NULL;
void createNode(char *dat){

    struct buffer *root, *temp;
    root=(struct buffer *)malloc(sizeof(struct buffer));
    root->data=dat;
    if(start==NULL){
    
        start=root;
        start->Next=NULL;
        
    }
    else{
    
        root->Next=start;
        start=root;
    }
   
  //  free(root);
    
}
void print(){
    struct buffer *cur;
    cur=start;
    while(cur!=0){
        printf("\n%s",cur->data);
        cur=cur->Next;
        
    }

}
int length()  
{  
  struct buffer *cur_ptr;  
  int count=0;  
  
  cur_ptr=start;  
  
  while(cur_ptr != NULL)  
  {  
     cur_ptr=cur_ptr->Next;  
     count++;  
  }  
  return(count);  
}
void addatpos(char *dat,int pos){

      int i;  
  struct buffer *temp, *prev_ptr, *cur_ptr;  
  
  cur_ptr=start;  
  
  if(pos > (length()+1) || pos <= 0)  
  {  
     printf("\nInsertion at given location is not possible\n ");  
  }
  else{
  
      if(pos==1){
      
          createNode(dat);
      }
        else  
      {  
          for(i=1;i<pos;i++)  
          {  
              prev_ptr=cur_ptr;  
              cur_ptr=cur_ptr->Next;  
          }  
  
          temp=(struct buffer *)malloc(sizeof(struct buffer));  
          temp->data=dat;  
  
          prev_ptr->Next=temp;  
          temp->Next=cur_ptr;  
      }
  
  
  
  }
  
}

int main(){
int i=length();
    char *data="nikhil";
    createNode(data);
    i=length();
      printf("\nl1 is %d",i);
  data="nik";
  createNode(data);
  i=length();
    printf("\nl2 is %d",i);
  data="overwrite";
  addatpos(data,3);
  print();
  i=length();
  printf("\nl3 is %d",i);
    return 0;
}

