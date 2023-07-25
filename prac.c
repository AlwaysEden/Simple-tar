#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int main()
{
  char buffer[100] = "Hello  world";
  printf("%lu", strlen(buffer));
  
}