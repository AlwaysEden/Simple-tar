#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <libgen.h>


typedef enum {
  archive = 0,
  list,
  extract,
  N_op
}cmd;

char * op_str[N_op] = {
  "archive",
  "list",
  "extract",
};

cmd 
get_op_code (char * s)
{
	for(int i = 0 ; i < N_op ; i++){
		if(strcmp(s, op_str[i]) == 0){
			return i;
		}
	}
	return N_op;
}

void _make_dir(char * path){
  struct stat dircheck;
  if(strcmp(path, ".") == 0)
    return;
  if(stat(path, &dircheck) == 0)
    return;

  char * pathcopy = strdup(path);
  char * parentpath = dirname(pathcopy);

  _make_dir(parentpath);
  mkdir(pathcopy, 0755);

  free(pathcopy);
}


void makedir(char * path){
  
  char * pathcopy = strdup(path);
  char * dirpath = dirname(pathcopy);
  _make_dir(dirpath);

  free(pathcopy);
}

void archivefiles(char * arc_filepath, char * src_path){
  
  struct dirent * entry;
  struct stat buffer;

  DIR * dp;
  if((dp = opendir(src_path)) == NULL){
    fprintf(stderr,"Error: Open the directory %s\n",src_path);
    exit(EXIT_FAILURE);
  }

  while( (entry = readdir(dp)) != NULL){

    char * new_filepath = malloc(strlen(src_path) + 1 + strlen(entry->d_name) + 1);
    sprintf(new_filepath, "%s/%s", src_path, entry->d_name);

    int status;
    if( (status = stat(new_filepath, &buffer)) == -1){
      fprintf(stderr, "Error: 1stat() faild%s\n",new_filepath);
      exit(EXIT_FAILURE);
    }

    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || (buffer.st_mode & S_IFMT) == S_IFLNK) continue;

    if((buffer.st_mode & S_IFMT) == S_IFREG){
      FILE * src_fd = fopen(new_filepath, "rb");
      FILE * des_fd = fopen(arc_filepath, "ab");
      if(src_fd == NULL || des_fd == NULL){
        fprintf(stderr,"Error: Impossible path: %s and %s\n", new_filepath, arc_filepath);
        exit(EXIT_FAILURE);
      }

      int pathlength = strlen(new_filepath);
      fwrite((void *)&pathlength, 1, sizeof(unsigned int), des_fd);
      fwrite(new_filepath, 1, pathlength, des_fd);
      fwrite((void *)&buffer.st_size, 1, sizeof(unsigned int), des_fd);

      char content[512];
      size_t read_check;
      while( (read_check = fread(content, 1, 512, src_fd)) ){
        char * towrite = content ;
        size_t written_acc = 0, written = 0 ;
        for (written_acc = 0 ; written_acc < read_check ; written_acc += written) {
          if ( (written = fwrite(towrite, 1, read_check -  written_acc, des_fd)) ) {
            break ;
          }
          towrite += written ;
          written_acc += written ;
        }
      }
      fclose(src_fd);
      fclose(des_fd);
    }else if((buffer.st_mode & S_IFMT) == S_IFDIR){
      archivefiles(arc_filepath, new_filepath);
    }
    free(new_filepath);
  }
  
  closedir(dp);
}

void listfile(char * arh_path){
  FILE * fd = fopen(arh_path, "r");

  size_t read_check;
  int buffer;
  while( (read_check = fread(&buffer, 1, 4, fd))){
    char * filepath = (char *)malloc(buffer + 1);
    read_check = fread(filepath, 1, buffer, fd);
    if(read_check != buffer){
      fprintf(stderr, "Error: Read file");
      exit(EXIT_FAILURE);
    }
    printf("%s\n", filepath);
    free(filepath);

    int filesize;
    read_check = fread(&filesize, 1, 4, fd);
    if(read_check != 4){
      fprintf(stderr, "Error: Read file");
      exit(EXIT_FAILURE);
    }

    char * filecontent = (char *)malloc(filesize + 1);
    read_check = fread(filecontent, 1, filesize, fd);
    free(filecontent);
  }
  fclose(fd);
}

void extract_arh(char * arc_filename){

  FILE * read_fd = fopen(arc_filename, "r+");
  if(read_fd == NULL){
    fprintf(stderr, "Error: Open archive file");
    exit(EXIT_FAILURE);
  }

  size_t read_check;
  int pathlength;
  while( (read_check = fread(&pathlength, 1, 4, read_fd) )){
    if(read_check != 4){
      fprintf(stderr, "Error: Read the pathlength of file.");
      exit(EXIT_FAILURE);
    }
    

    char * filepath = (char *)malloc(pathlength + 1);
    read_check = fread(filepath, 1, pathlength, read_fd);
    if(read_check != pathlength){
      fprintf(stderr, "Error: Read the filepath");
      exit(EXIT_FAILURE);
    }

    makedir(filepath);

    int filesize;
    read_check = fread(&filesize, 1, 4, read_fd);
    if(read_check != 4){
      fprintf(stderr, "Error: Read the filesize");
      exit(EXIT_FAILURE);
    }

      
    FILE * write_fd = fopen(filepath,"w+");
    char filedata[512];
    int read_size = filesize < 512 ? filesize : 512;
    int read_count = filesize < 512 ? 1: filesize/512 + 1;
    for(int i = 0; i < read_count; i++){
      read_check = fread(filedata, 1, read_size, read_fd);
      if(read_check != read_size || filesize == 0){
        break;
      }
      char * towrite = filedata ;
      size_t written_acc = 0, written = 0 ;
      for (written_acc = 0 ; written_acc < read_check ; written_acc += written) {
        if ( (written = fwrite(towrite, 1, read_check -  written_acc, write_fd)) ) {
          break ;
        }
        towrite += written ;
        written_acc += written ;
      }
      filesize -= read_size;
      if(filesize < read_size){
        read_size = filesize;
      }
    }
    free(filepath);
    fclose(write_fd);
  }
  fclose(read_fd);

}

int main(int argc, char * args[]){
  int op;
  args[1] = "extract";
  args[2] = "achfile";
  switch(op = get_op_code(args[1])){

    case archive:
      archivefiles(args[2], args[3]);
      break;
    case list:
      listfile(args[2]);
      break;
    case extract:
      extract_arh(args[2]);
      break;
    default:
      printf("No command.\n");
  }

}