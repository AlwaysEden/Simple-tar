#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void copyfiles(char *srcpath, char * despath){
  DIR * srcdir;
  DIR * desdir;

  struct dirent *entry;
  struct stat src_buffer;
  struct stat des_buffer;
  int status;

  if((srcdir = opendir(srcpath)) == NULL){ //Check if srcpath exist and open the directory to copy it.
    fprintf(stderr,"Error: Open the source directory %s\n",srcpath);
    exit(EXIT_FAILURE);
  }

  while( (entry = readdir(srcdir)) != NULL){

    char *new_srcpath = malloc(strlen(srcpath) + 1 + strlen(entry->d_name) +1);
    sprintf(new_srcpath, "%s/%s", srcpath, entry->d_name);
    
    if( (status = lstat(new_srcpath, &src_buffer)) == -1){
      fprintf(stderr, "Error: stat() faild\n");
      exit(EXIT_FAILURE);
    }


    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || (src_buffer.st_mode & S_IFMT) == S_IFLNK) continue;

    char * new_despath = malloc(strlen(despath) + 1 + strlen(entry->d_name) +1);
    sprintf(new_despath, "%s/%s", despath, entry->d_name);

    if((src_buffer.st_mode & S_IFMT) == S_IFDIR){
  
      if( (status = lstat(despath, &des_buffer)) == -1){
        fprintf(stderr, "Error: stat() Error\n");
        exit(EXIT_FAILURE);
      }
      if(src_buffer.st_ino == des_buffer.st_ino){
        continue;
      }
      mkdir(new_despath, 755);
      chmod(new_despath, src_buffer.st_mode);
      copyfiles(new_srcpath, new_despath);
    }else if((src_buffer.st_mode & S_IFMT) == S_IFREG){

      FILE * src_fd = fopen(new_srcpath, "rb");
      FILE * des_fd = fopen(new_despath, "wb+");
      if(src_fd == NULL || des_fd == NULL){
        fprintf(stderr,"Error: Impossible new path %s and %s\n", new_srcpath, new_despath);
        perror("helo");
        exit(EXIT_FAILURE);
      }
      char buffer[512];
      size_t read_check;
      while( (read_check = fread(buffer, 1, 512, src_fd)) ){
        char * towrite = buffer ;
        size_t written_acc = 0, written = 0 ;
        for (written_acc = 0 ; written_acc < read_check ; written_acc += written) {
          if ( (written = fwrite(towrite, 1, read_check -  written_acc, des_fd)) ) {
            break ;
          }
          towrite += written ;
          written_acc += written ;
        }
      }
      chmod(new_despath, src_buffer.st_mode);
      fclose(src_fd);
      fclose(des_fd);
    }
    // else if((src_buffer.st_mode & S_IFMT) == S_IFLNK){
    //   if(!symlink(new_srcpath, new_despath)){
    //     fprintf(stderr, "Error: Symbolic link copy");
    //     exit(EXIT_FAILURE);
    //   }
    // }
    free(new_srcpath);
    free(new_despath);
  }
  
  closedir(srcdir);
}


int main(int argc, char * args[]){

  copyfiles(args[1], args[2]);
  // copyfiles(args[1], args[2]);
  // recursionDir("./num1/num2");

}