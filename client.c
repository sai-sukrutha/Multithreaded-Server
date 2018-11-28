// *
 // * client.c, copyright 2001 Steve Gribble
 // *
 // * The client is a single-threaded program; it sits in a tight
 // * loop, and in each iteration, it opens a TCP connection to
 // * the server, sends a request, and reads back a response.
 

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include "SocketLibrary/socklib.h"
#include "common.h"

/**
 * This program should be invoked as "./client hostname portnumber",
 * for example, "./client spinlock 4342".
 */

int main(int argc, char **argv) {
  int  socket_talk, i;
  // REQUEST_SIZE=3;
  char request[REQUEST_SIZE];
  char response[RESPONSE_SIZE];

  if (argc != 3) {
    fprintf(stderr,
	    "(CLIENT): Invoke as  'client machine.name.address socknum'\n");
    exit(1);
  }

  // initialize request to some silly data
  
  // for (i=0; i<REQUEST_SIZE; i++) {
    // request[i] = (char) i%255;
    // printf("%c\n",(char) i%255 );
  // }
  // request[0]='h';
  // request[1]='i';
  // request[2]='\0';
   printf("Client data: \n");
    scanf("%s",&request);
    printf("%s",request);

  printf("\n");
  // spin forever, opening connections, and pushing requests
  while(1) {
   

    int result;

    // open up a connection to the server
    if ((socket_talk = sconnect(argv[1], argv[2])) < 0) {
      perror("(CLIENT): sconnect");
      exit(1);
    }

    // write the request
    result = correct_write(socket_talk, request, REQUEST_SIZE);
    if (result == REQUEST_SIZE) {
      // read the response
      result = correct_read(socket_talk, response, RESPONSE_SIZE);
      printf("Recieved response %s\n",response );
    }
    close(socket_talk);
  }
  
  return 0;
}
