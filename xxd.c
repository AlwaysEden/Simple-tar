#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


int main()
{
  FILE *fd;
  if(!(fd = fopen("board2.txt","r"))){
    fprintf(stderr,"Error: Open the file\n");
    return 1;
  }

  size_t count;
  size_t size;
  char *buffer;
  int index = 0;


  while(!feof(fd)){
    buffer = malloc(sizeof(char) * 512);
    size_t op = fread(buffer, 1, 512, fd);
    if(!op){
      if(feof(fd)){
        free(buffer);
        break;
      }
      while(!feof(fd)){
        op = fread(buffer,1, 512, fd);
      }
    }
    
    int n = op%16 == 0? op/16: op/16+1;

    for(int i = 0; i < n; i++){
      for(int j = 0; j < 16; j ++){
        if(j%2==0){
          printf(" ");
        }
        printf("%02x", buffer[index]);
        index++;
      }
      printf("\n");
    }
  }
}