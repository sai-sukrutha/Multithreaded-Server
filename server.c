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
#include <sys/stat.h>
#include <sys/types.h>
#include<sys/sendfile.h>
#include<fcntl.h>
// #include "SocketLibrary/socklib.h"
// #include "SocketLibrary/saccept.c"
#include "common.c"
#include "threadpool.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include<vector>
#include<set>
#include<map>
#include<bits/stdc++.h>
using namespace std;
using std::map;
extern int errno;

int   setup_listen(char *socketNumber);
char *read_request(int fd);
char *process_request(char *request, int *response_length);
void  send_response(int fd, char *response, int response_length);
void serve_request(void* fd);

//map for content-type and file type
map <string,string> get_mime_types(){
    map <string,string> mime_types;
    // mime_types.insert(std::pair<string,string>("jpg","image/jpg"));
    mime_types["jpg"]="image/jpg";
    // mime_types["jpg"]="text/html; charset=utf-8";
    mime_types["jpeg"]="image/jpg";
    mime_types["css"]="text/css";
    mime_types["js"]="application/javascript";
    mime_types["json"]="application/json";
    mime_types["txt"]="text/plain";
    mime_types["gif"]="image/gif";
    mime_types["png"]="image/png";
    mime_types["mp4"]="video/mp4";
    mime_types["html"]="text/html";
    mime_types["htm"]="text/html";
    mime_types["mkv"]="video/x-matroska";
    mime_types["woff2"]="application/font-woff2";
    mime_types["map"]="application/x-navimap";
    mime_types["pdf"]="application/pdf";
    return mime_types;
}


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
    dispatch(tp, serve_request, (void *) socket_talk,0);
  }
  destroy_threadpool(tp);
}

void serve_request(void* fd){
  int socket_talk=reinterpret_cast<intptr_t>(fd);
  int request_len;
  char request[REQUEST_SIZE];
  
  request_len=recv(socket_talk,request,REQUEST_SIZE,0);
  if (request_len!=0) {
    int response_length;
    string str(request);
    vector <string> tokens; 
    stringstream check1(str); 
      
    string intermediate; 
    // Tokenizing w.r.t. space ' ' 
    while(getline(check1, intermediate, ' ')) 
    { 
        tokens.push_back(intermediate); 
    } 
    int f=0;
    int size=0;
    char * token=new char[tokens[1].length()];
    strcpy(token,(tokens[1].substr(1)).c_str());
    token[tokens[1].length()-1]='\0';
    // printf("str:%s\n",token);
    string fname(token);
    string ext=fname.substr(fname.find_last_of(".") + 1);
    // printf("ext: %s\n",ext.c_str());
    map<string,string> mime_types=get_mime_types();
    string content_type=mime_types[ext];
    // printf("ext: %s content_type: %s\n",ext.c_str(),content_type.c_str());
      char response[RESPONSE_SIZE];
    if(strcmp(token,"favicon.ico")!=0)
    {
   
        f = open(token,O_RDONLY);
        if (f == -1 ) {
          token="error.html";
          f=open("error.html",O_RDONLY);
          content_type="text/html";
           
        }
  
        struct stat st; 
        if (stat(token, &st) == 0)
              size= st.st_size;
 
        //response header
        char test[] = "HTTP/1.1 200 OK\nContent-Type: ";
        char *contentType=new char[content_type.length()+1];
        strcpy(contentType, content_type.c_str());
        contentType[content_type.length()]='\0';
        strcat(test,contentType);
        char test2[]="\nContent-Length: ";
        strcat(test,test2);
        char test1[10];
        sprintf(test1,"%d\n\n",size);
        strcat(test,test1);
        bzero(response,sizeof(response));
        strcpy(response,test);
    }
    
    response_length=strlen(response);
    if (response != NULL) {
      send(socket_talk,response,response_length,0);
      off_t offset = 0L;
      if(sendfile(socket_talk, f, &offset, size) == -1) {
        perror("send file");
        close(f);
      }
      bzero(request,sizeof(request));
    }
  }
  // sleep(10);
  close(socket_talk); 
  
}


