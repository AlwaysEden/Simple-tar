#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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


void makedir(char * path){
  char * filename = strrchr(path, '/');
  char * new_path = (char*)malloc(strlen(path));
  strncpy(new_path, path, strlen(path) - strlen(filename));

  char * delim = "/";
  char * token = strtok(new_path, delim);
  char * dirpath = (char *)malloc(strlen(new_path)+1);

  while(token != NULL){
    token = strtok(NULL, delim);
    sprintf(dirpath, "%s/%s", dirpath, token);
    printf("%s", dirpath);
    mkdir(dirpath, 755);
  }
  free(new_path);
  free(dirpath);
}

void archivefiles(char * arc_filepath, char * src_path){
  DIR * dp;
  struct dirent *entry;
  struct stat buffer;
  int status;

  if((dp = opendir(src_path)) == NULL){
    fprintf(stderr,"Error: Open the directory %s\n",src_path);
    exit(EXIT_FAILURE);
  }

  while( (entry = readdir(dp)) != NULL){

    char *new_filepath = malloc(strlen(src_path) + 1 + strlen(entry->d_name)+1);
    strcat(new_filepath, src_path);
    strcat(new_filepath, "/");
    strcat(new_filepath, entry->d_name);

    if( (status = stat(new_filepath, &buffer)) == -1){
      fprintf(stderr, "Error: 1stat() faild%s\n",new_filepath);
      exit(EXIT_FAILURE);
    }

    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || (buffer.st_mode & S_IFMT) == S_IFLNK) continue;

  
    if((buffer.st_mode & S_IFMT) == S_IFDIR){
      archivefiles(arc_filepath, new_filepath);
    }else if((buffer.st_mode & S_IFMT) == S_IFREG){
      FILE * src_fd = fopen(new_filepath, "rb");
      FILE * des_fd = fopen(arc_filepath, "ab");
      if(src_fd == NULL || des_fd == NULL){
        fprintf(stderr,"Error: Impossible path: %s and %s\n", new_filepath, arc_filepath);
        exit(EXIT_FAILURE);
      }
      //파일 이름 길이를 아이노드에서 발견할 수 있도록 array copy
      /*
        1. The length of file path.
        2. A String of file path.
        3. The size of file data.
      */
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
      // chmod(arc_filepath, buffer.st_mode);
      fclose(src_fd);
      fclose(des_fd);
      printf("%s\n", new_filepath);
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
    char filepath[buffer];
    read_check = fread(filepath, 1, buffer, fd);
    if(read_check != buffer){
      fprintf(stderr, "Error: Read file");
      exit(EXIT_FAILURE);
    }
    printf("%s\n", filepath);

    int filesize;
    read_check = fread(&filesize, 1, 4, fd);
    if(read_check != 4){
      fprintf(stderr, "Error: Read file");
      exit(EXIT_FAILURE);
    }

    char filecontent[filesize];
    read_check = fread(filecontent, 1, filesize, fd);
  }
}


void extract_arh(char * arc_filename){
  DIR * srcdir;
  DIR * desdir;

  struct dirent *entry;
  struct stat src_buffer;
  struct stat des_buffer;
  int status;

  /*
    파일을 4바이트 읽고, 그 다음 4바이트가 가르키는 정수의 길이 만큼 읽고, 그 다음 4바이트만큼 읽고, 그 다음 4바이크가 가르키는 파일 사이즈만큼 데이터를 읽으면 한 파일을 읽는 건 끝. 이걸 파일 끝날때마다 계속.
  */
  FILE * arc = fopen(arc_filename, "r+");
  if(arc == NULL){
    fprintf(stderr, "Error: Open archive file");
    exit(EXIT_FAILURE);
  }

  size_t read_check;
  int pathlength;
  while( (read_check = fread(&pathlength, 1, 4, arc) )){
    if(read_check != 4){
      fprintf(stderr, "Error: Read the file");
    }
    
    // printf("%d ", pathlength); 

    char * filepath = (char *)malloc(pathlength);
    read_check = fread(filepath, 1, pathlength, arc);
    if(read_check != pathlength){
      fprintf(stderr, "Error: Read the file");
    }

    // printf("%s ", filepath);

    int filesize;
    read_check = fread(&filesize, 1, 4, arc);
    if(read_check != 4){
      fprintf(stderr, "Error: Read the file");
    }
    // printf("%d ", filesize);

    makedir(filepath);
    // FILE * createfile = fopen(filepath, "w+");

    char * filedata = (char *)malloc(filesize);
    fread(filedata, 1, filesize, arc);
    // printf("%s\n",filedata);
    //file사이즈가 겁나 클 수도 있는데 이걸 막 할당해준다? 읽으면서 점차적으로 할당해주는 건 어때
  }


}

int main(int argc, char * args[]){
  int op;
  // copyfiles(args[1], args[2]);
  // recursionDir("./num1/num2");
  args[1] = "list";
  args[2] = "ach";
  args[3] = "./num1/num2";
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
  }

}