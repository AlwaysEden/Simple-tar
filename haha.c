#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


void copyfiles(char *copiedpath, char * despath){
  DIR * dp;
  struct dirent *entry;
  struct stat buffer;
  int status;

  if((dp = opendir(copiedpath)) == NULL){
    fprintf(stderr,"Error: Open the directory %s\n",copiedpath);
    exit(EXIT_FAILURE);
  }

  while( (entry = readdir(dp)) != NULL){

    char *filepath = malloc(strlen(copiedpath) + 1 + strlen(entry->d_name) +1);
    sprintf(filepath, "%s/%s", copiedpath, entry->d_name);
    // strcat(filepath, copiedpath);
    // strcat(filepath, "/");
    // strcat(filepath, entry->d_name);
    if(strcmp(filepath, "./num1/.DS_Store") == 0) continue;
    if( (status = stat(filepath, &buffer)) == -1){
      fprintf(stderr, "Error: lstat() faild\n");
      exit(EXIT_FAILURE);
    }


    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || (buffer.st_mode & S_IFMT) == S_IFLNK) continue;
    if((buffer.st_mode & S_IFMT) == S_IFDIR){
      // copyfiles(filepath);
    }else if((buffer.st_mode & S_IFMT) == S_IFREG){
      
      char * new_despath = malloc(strlen(despath) + 1 + strlen(entry->d_name) +1);
      sprintf(new_despath, "%s/%s", despath, entry->d_name);
      // strcat(new_despath, despath);
      // strcat(new_despath, "/");
      // strcat(new_despath, entry->d_name);

      FILE * copied_fd = fopen(filepath, "rb");
      FILE * des_fd = fopen(new_despath, "wb+");

      char buffer[512];
      size_t read_check;
      while(read_check = fread(buffer, 1, 512, copied_fd) ){
        char * towrite = buffer ;
        size_t written_acc = 0, written = 0 ;
        for (written_acc = 0 ; written_acc < read_check ; written_acc += written) {
          if (written = fwrite(towrite, 1, read_check -  written_acc, des_fd)) {
            break ;
          }
          towrite += written ;
          written_acc += written ;
        }
        
        free(new_despath);
        fclose(copied_fd);
        fclose(des_fd);
      }
    free(filepath);
    }
  }
  
  closedir(dp);
}


void recursionDir(char *path) {
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


    if(!strncmp(entry->d_name, ".", 1) || (buffer.st_mode & S_IFMT) == S_IFLNK) continue;

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
  copyfiles("./num1", "./num1/num2");
  // copyfiles(args[1], args[2]);
  // recursionDir("./num1/num2");

}