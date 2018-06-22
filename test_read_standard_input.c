#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define BUFFER_SIZE 1024

int   main(void)
{
  char  buffer[BUFFER_SIZE + 1];
  char  standard_input[10000];
  int   index = 0;
  size_t  l;
  //Boucle infinie de lecture sur l'entrée standard et le fifo
  while(1)
  {
    l = read(0, buffer, BUFFER_SIZE);
    if (l > 0)
    {
      printf("Je read\n");
      buffer[l] = '\0';
      strcpy(&standard_input[index], buffer);
      index += l;
      //Regarder s'il y a un \n pour lancer
      if (buffer[0] == '\n')
        printf("%s\n", standard_input);
    }
    //Récupérer l'entrée standard
    //Lire sur fd_c
  }
}
