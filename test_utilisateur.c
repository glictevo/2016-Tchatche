#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define S "/tmp/pipe_s"

int main(void) {
  int d1 = open(S, O_WRONLY);
  if (d1==-1) { perror("ouverture t1"); exit(1); }
  printf("ouverture t1 ok\n");

  char *message = "0036HELO0010NeoDereshi0010NeoDereshi\0";

  write(d1, message, strlen(message));

  close(d1);
}
