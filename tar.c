#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
int main()
{
  FILE *fd;
  if(!(fd = fopen("board2.txt","r"))){
    fprintf(stderr,"Error: Open the file\n");
    return 1;
  }

  size_t count;
  size_t size;
  char * buffer;

  fseek (fd , 0 , SEEK_END);
  count = ftell (fd);
  rewind (fd);

  buffer = malloc(sizeof(char) * count);

  if(count != fread(buffer, 512, count, fd)){
    fprintf(stderr, "Error: Read the file.\n");
    return 1;
  }


  unsigned int addr;

  // for(int i = 0; i < count; i++ ){
  //   printf("%c ",buffer[i]);
  // }
  // printf("%zu", count);

  int offset = 0;
  int output_line_count = count%16 > 0? count/16+1 : count/16;
  int index_for_16 = 0;
  int index_for_ascii = 0;

  for(int i = 0; i < output_line_count; i++){
    printf("%08x:", offset);
    offset += 16;
    for(int j = 0; j < 16; j++){
      if(j%2 == 0){
        printf(" ");
      }
      printf("%x",buffer[index_for_16]);
      index_for_16++;
    }

    
    for(int i = 0; i < 16; i++){

      if(!iscntrl(buffer[index_for_ascii])){
        printf("%c",buffer[index_for_ascii]);
      }
      else{
        printf(".");
      }
      
      index_for_ascii++;
    }
    printf("\n");
  }




  
}