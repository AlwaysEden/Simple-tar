#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


void recursionDir(char *path){
  DIR * dp;
  struct dirent *entry;
  struct stat buffer;
  int status;

  if((dp = opendir(path)) == NULL){
    fprintf(stderr,"Error: Open the directory %s\n",path);
    exit(EXIT_FAILURE);
  }

  while( (entry = readdir(dp)) != NULL){

    char *filepath = malloc(strlen(path) + 1 + strlen(entry->d_name));
    strcat(filepath, path);
    strcat(filepath, "/");
    strcat(filepath, entry->d_name);

    if( (status = stat(filepath, &buffer)) == -1){
      fprintf(stderr, "Error: stat() faild\n");
      exit(EXIT_FAILURE);
    }

    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || (buffer.st_mode & S_IFMT) == S_IFLNK) continue;

  
    if((buffer.st_mode & S_IFMT) == S_IFDIR){
      recursionDir(filepath);
    }else if((buffer.st_mode & S_IFMT) == S_IFREG){
      printf("%s\n", filepath);
    }
    free(filepath);
  }
  
  closedir(dp);
}



int main(int argc, char * args[]){

  DIR * dp;
  struct dirent *entry;
  args[1] = "num1";
  printf("%s\n",args[1]);
  recursionDir(args[1]);


}