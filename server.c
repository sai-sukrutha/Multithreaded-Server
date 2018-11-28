  /**
 * server.c, copyright 2001 Steve Gribble
 *
 * The server is a single-threaded program.  First, it opens
 * up a "listening socket" so that clients can connect to
 * it.  Then, it enters a tight loop; in each iteration, it
 * accepts a new connection from the client, reads a request,
 * computes for a while, sends a response, then closes the
 * connection.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <cstdint>
#include <string>
#include <stdlib.h>
// #include "SocketLibrary/socklib.h"
// #include "SocketLibrary/saccept.c"
#include "common.c"
#include "threadpool.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include<vector>
#include<bits/stdc++.h>
using namespace std;
extern int errno;

int   setup_listen(char *socketNumber);
char *read_request(int fd);
char *process_request(char *request, int *response_length);
void  send_response(int fd, char *response, int response_length);
void serve_request(void* fd);

/**
 * This program should be invoked as "./server <socketnumber>", for
 * example, "./server 4342".
 */

int main(int argc, char **argv)
{
  char buf[1000];
  int  socket_listen;
  int  socket_talk;
  int  dummy, len;

  if (argc != 3)
  {
    fprintf(stderr, "(SERVER): Invoke as  './server socknum threadnum'\n");
    fprintf(stderr, "(SERVER): for example, './server 4434 10'\n");
    exit(-1);
  }

  /* 
   * Set up the 'listening socket'.  This establishes a network
   * IP_address:port_number that other programs can connect with.
   */
  struct sockaddr_in address,client_address;
  socket_listen=socket(AF_INET, SOCK_STREAM, 0);
  if (socket_listen==0)
  {
    /* code */
    perror("socket creation failed.");
    exit(-1);
  }

  address.sin_family=AF_INET;
  address.sin_addr.s_addr=INADDR_ANY;
  address.sin_port=htons(atoi(argv[1]));
  if(bind(socket_listen,(struct sockaddr *)&address,sizeof(address))<0){

    perror("socket bind failed.");
    exit(-1);
  }
  // socket_listen = setup_listen(argv[1]);
  if(listen(socket_listen,10)<0)
  {
    perror("Listen error.");
    exit(-1);
  }


    printf("listening..\n");

  /* 
   * Here's the main loop of our program.  Inside the loop, the
   * one thread in the server performs the following steps:
   * 
   *  1) Wait on the socket for a new connection to arrive.  This
   *     is done using the "accept" library call.  The return value
   *     of "accept" is a file descriptor for a new data socket associated
   *     with the new connection.  The 'listening socket' still exists,
   *     so more connections can be made to it later.
   *
   *  2) Read a request off of the listening socket.  Requests
   *     are, by definition, REQUEST_SIZE bytes long.
   *
   *  3) Process the request.
   *
   *  4) Write a response back to the client.
   *
   *  5) Close the data socket associated with the connection
   */
  threadpool tp;

  tp = create_threadpool(atoi(argv[2]));
  socklen_t client_len;
  while(1) {
    

     // step 1
    socket_talk=accept(socket_listen,(struct sockaddr *)&client_address,&client_len);
    if (socket_talk < 0) {
      fprintf(stderr, "An error occured in the server; a connection\n");
      fprintf(stderr, "failed because of ");
      perror("");
      exit(1);
    }
    printf("accepted..\n");
   
    dispatch(tp, serve_request, (void *) socket_talk);
   
  }
  destroy_threadpool(tp);
}

void serve_request(void* fd){
// printf("Hi..I am in serve request!!\n");
  int socket_talk=reinterpret_cast<intptr_t>(fd);
  int request_len;
  // printf("Socket Talk%d\n",socket_talk );
  char request[REQUEST_SIZE];
  
  //request = read_request(socket_talk);  // step 2
  request_len=recv(socket_talk,request,REQUEST_SIZE,0);

  printf("Data recieved: %s\n",request );
  if (request != NULL) {
    int response_length;
    string str(request);
    vector <string> tokens; 
      
    // stringstream class check1 
    stringstream check1(str); 
      
    string intermediate; 
    // Tokenizing w.r.t. space ' ' 
    while(getline(check1, intermediate, ' ')) 
    { 
        tokens.push_back(intermediate); 
    } 
     
    char * token=new char[tokens[1].length()];
    strcpy(token,(tokens[1].substr(1)).c_str());
    token[tokens[1].length()-1]='\0';
    printf("str:%s\n",token);
      char response[RESPONSE_SIZE];
    if(strcmp(token,"favicon.ico")!=0){
    FILE *f = fopen(token, "rb");
    if(f==NULL){
      f=fopen("error.html","rb");
    }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);  //same as rewind(f);

  char *string = new char[fsize + 1];
  fread(string, fsize, 1, f);
  fclose(f);
  string[fsize]='\0';
  char test[] = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
  // strcat(test,itoa(fsize));
  char test1[10];
  sprintf(test1,"%d\n\n",fsize);
  strcat(test,test1);
  strcat(test,string);

  // char response[RESPONSE_SIZE]=  "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 85\n\n<!DOCTYPE html><html><head></head><body><h1 align='centre'>Welcome to my Website!</h1></body></html>";;

  strcpy(response,test);
}
    // response = process_request(request, &response_length);  // step 3
    response_length=strlen(response);
    if (response != NULL) {
      //send_response(socket_talk, response, response_length);  // step 4
      send(socket_talk,response,response_length,0);
      // printf("Response: %s\n",response );
    }
  }
  sleep(50);
  close(socket_talk); 
  // if (request != NULL)
  //   free(request);
  // if (response != NULL)
  //   free(response);
    
}


/**
 * This function accepts a string of the form "5654", and opens up
 * a listening socket on the port associated with that string.  In
 * case of error, this function simply bonks out.
 */

// int setup_listen(char *socketNumber) {
//   int socket_listen;

//   if ((socket_listen = slisten(socketNumber)) < 0) {
//     perror("(SERVER): slisten");
//     exit(1);
//   }

//   return socket_listen;
// }

/**
 * This function reads a request off of the given socket.
 * This function is thread-safe.
 */

char *read_request(int fd) {
  char *request = (char *) malloc(REQUEST_SIZE*sizeof(char));
  int   ret;
  printf("In read request..\n");
  // cout<"In read request.."<<endl;
  if (request == NULL) {
    fprintf(stderr, "(SERVER): out of memory!\n");
    exit(-1);
  }
  // printf("In read request..\n");

  ret = correct_read(fd, request, REQUEST_SIZE);
  // cout<<"ret: "<<ret<<endl;
  printf("ret: %d\n", ret);
  printf("stat: %s\n", request);
  if (ret != REQUEST_SIZE) {
    free(request);
    request = NULL;
  }
  return request;
}

/**
 * This function crunches on a request, returning a response.
 * This is where all of the hard work happens.  
 * This function is thread-safe.
 */

#define NUM_LOOPS 500000000

char *process_request(char *request, int *response_length) {
  char *response = (char *) malloc(RESPONSE_SIZE*sizeof(char));
  int   i,j;

  // just do some mindless character munging here

  for (i=0; i<RESPONSE_SIZE; i++)
    response[i] = request[i%REQUEST_SIZE];

  for (j=0; j<NUM_LOOPS; j++) {
    for (i=0; i<RESPONSE_SIZE; i++) {
      char swap;

      swap = response[((i+1)%RESPONSE_SIZE)];
      response[((i+1)%RESPONSE_SIZE)] = response[i];
      response[i] = swap;
    }
  }

  *response_length = RESPONSE_SIZE;
  return response;
}

