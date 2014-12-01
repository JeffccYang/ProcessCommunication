#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>

//#define USE_UUID 
#ifdef USE_UUID
  #include <uuid/uuid.h>
#endif

#include <netdb.h>

#include "processcom.h"

// #define USE_ZMQ 

#ifdef USE_ZMQ
  #include <assert.h>  
  #include <zmq.h>  
#endif

#define MAX_BUF_SIZE  (512*1024)
 
static int ServerFlag = 1;


#ifdef USE_ZMQ

  void z_server()
  { 
    // Socket to talk to clients  
    void *context = zmq_ctx_new ();  
    void *responder = zmq_socket (context, ZMQ_REP);  
    int rc = zmq_bind (responder, "tcp://*:5555");  
    assert (rc == 0);  
  
    while (1) {  
        char buffer [10];  
        zmq_recv (responder, buffer, 10, 0);  
        printf ("Received Hello\n");  
        zmq_send (responder, "World", 5, 0);  
        sleep (1); // Do some 'work'  
    }  
  
  }

  void z_client()
  {
    printf ("Connecting to hello world server…\n");  
    void *context = zmq_ctx_new ();  
    void *requester = zmq_socket (context, ZMQ_REQ);  
    zmq_connect (requester, "tcp://localhost:5555");  
  
    int request_nbr;  
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {  
        char buffer [10];  
        printf ("Sending Hello %d…\n", request_nbr);  
        zmq_send (requester, "Hello", 5, 0);  
        zmq_recv (requester, buffer, 10, 0);  
        printf ("Received World %d\n", request_nbr);  
    }  
    zmq_close (requester);  
    zmq_ctx_destroy (context);  
  }
 

#endif


void server_stop()
{
  ServerFlag = 0;
}

static int first_handler(int fd, char* buf)
{
  int connected_fd;
  struct sockaddr_un from;
  int lenfrom = sizeof(from);
  
  bzero(&from, sizeof(from));
  
  if ((connected_fd = accept(fd, (struct sockaddr *)&from, &lenfrom)) < 0 )
  {
    printf("accept failed\n");
    return -1;
  }
 
  int nRecvSize = 0;

  nRecvSize = recv(connected_fd, buf, MAX_BUF_SIZE, 0);
  if ( nRecvSize < 0)
  {
    printf("recv failed\n");
    return -1;
  }
 
  printf("recv %d\n",nRecvSize);

  close(connected_fd);
 
  return nRecvSize;
}
 
int server_create_listener(char* name,  funCB funp)
{
  int fd, ret=-1, bufSize;
  struct sockaddr_un saddr;
  struct sockaddr_un from;
  char buf[MAX_BUF_SIZE];
 
  /* open a socket */
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    printf("socket failed\n");
    goto fun_exit;
  }
  
  /* set the addr */
  unlink(name);
  bzero(&saddr, sizeof(saddr));
  saddr.sun_family = AF_UNIX;
  strcpy(saddr.sun_path, name);
 
  /* bind socket with addr */
  if (bind(fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
  {
    printf("bind failed\n");
    goto fun_exit;
  }
  
  /* listen fd */
  if (listen(fd, 10) < 0)
  {
    printf("listen failed\n");
    goto fun_exit;
  }
 
  fd_set rset;
  FD_ZERO(&rset);
  FD_SET(fd, &rset);
 
  struct  timeval timeout;

  while (ServerFlag)
  {
    timeout.tv_sec = SERVER_CHECK_TIMEOUT; 
    timeout.tv_usec = 0; 

    FD_ZERO(&rset); 
    FD_SET(fd,&rset); 

    int reveal = select(fd+1, &rset, NULL, NULL, &timeout);

    if (reveal <= 0) 
    { 
         printf("<server timeout>(%s)\n",name);
       // quitProgram(number); /* error or connection timed out */
    } 
    else{ 
      if (FD_ISSET(fd, &rset))
      {
        bufSize = first_handler(fd, buf);
        funp((unsigned char *)buf,bufSize);
      }
    }

  }
  ret = 0;
 
fun_exit:
   printf("listen failed\n");
  if (fd>0) close(fd);
  return ret;
}
 
int client_send_to_listener(char* app_name, void* buf, int buf_size)
{
  int fd, ret=-1;
  struct sockaddr_un saddr;
 
  if (buf_size > MAX_BUF_SIZE)
  {
    printf("The max message size is %d\n", MAX_BUF_SIZE);
    goto fun_exit;
  }
 
  /* open a socket */
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    printf("socket failed\n");
    goto fun_exit;
  }
  
  /* set the saddr */
  bzero(&saddr, sizeof(saddr));
  saddr.sun_family = AF_UNIX;
  strcpy(saddr.sun_path, app_name);
 
  /* connect to server */
  if (connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
  {
    printf("connect failed\n");
    goto fun_exit;
  }
 
  /* send data */
  if (send(fd, buf, buf_size, 0) < 0)
  {
    printf("send failed\n");
    goto fun_exit;
  }
  ret = 0;
 
fun_exit:
  if (fd>0) close(fd);
  return ret;
}

//#define DEFAULT_LISTEN_PORT   "1234"

//#define DEFAULT_SERVER_IP   "127.0.0.1"
//#define DEFAULT_SERVER_PORT   "1234"

//typedef struct {
 // int sockfd;
 // struct sockaddr_in theiraddr;
//} conn_t;

int server_create_listener_net(char* port, funCB funp)
{

  int fd=0, ret=-1, bufSize;
  char buf[MAX_BUF_SIZE];
//------------------------------------------------------
  int yes=1;
  struct addrinfo hints;
  struct addrinfo *result = NULL, *p;
//  conn_t* listenconn = (conn_t*)malloc(sizeof(conn_t));  // listen on sock_fd, new connection on new_fd

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  
  int status = getaddrinfo(NULL, port, &hints, &result);
  if (status != 0) {
    printf("getaddrinfo error: %s\n", gai_strerror(status));
    goto fun_exit;
  }
  
  // loop through all the results and bind to the first we can
  for(p = result; p != NULL; p = p->ai_next) {
    if ((fd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
        printf("could not create socket with this address, retrying...\n");
        continue;
       }
       if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes,
          sizeof(int)) == -1) {
          printf("setsockopt on this socket failed, retrying...\n");
          continue;//return -1;
       }
       if (bind(fd, p->ai_addr, p->ai_addrlen) == -1) {
          close(fd);
          printf("bind on this address failed, retrying...\n");
          continue;
       }
        break;
  }

  if (p == NULL) {
    fprintf(stderr, "failed to bind\n");
     goto fun_exit;
  }

  freeaddrinfo(result); // all done with this structure

  if (listen(fd, 10) == -1) {
    perror("could not listen\n");
    goto fun_exit;
  }
//------------------------------------------------------  
  fd_set rset;
  FD_ZERO(&rset);
  FD_SET(fd, &rset);
 
  while (1)
  {
    select(fd+1, &rset, NULL, NULL, NULL);
    if (FD_ISSET(fd, &rset))
    {
      bufSize = first_handler(fd, buf);

      //printf("<< %s,%d >>\n", buf, bufSize);
       funp((unsigned char *)buf,bufSize);
    }
  }
  ret = 0;
//------------------------------------------------------

fun_exit:
  printf("listen failed\n");
  if (fd>0) close(fd);
  return ret;
}


int client_send_to_listener_net(char* server_ip, char* port,  void* buf, int buf_size)
{
  int fd, ret=-1;
  if (buf_size > MAX_BUF_SIZE)
  {
    printf("The max message size is %d\n", MAX_BUF_SIZE);
    goto fun_exit;
  }

//------------------------------------------------------
  struct addrinfo hints;
  struct addrinfo *result = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

 // conn_t* serverconn = (conn_t*)malloc(sizeof(conn_t));
  
  int status = getaddrinfo(server_ip, port, &hints, &result);
  if (status != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return NULL;
  }
  
  if ((fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0)
  {
    printf("socket failed\n");
    goto fun_exit;
  }
 
  status = connect(fd, result->ai_addr, result->ai_addrlen);

  /* connect to server */
  if (status < 0)
  {
    printf("connect failed\n");
    goto fun_exit;
  }
 
  /* send data */
  if (send(fd, buf, buf_size, 0) < 0)
  {
    printf("send failed\n");
    goto fun_exit;
  }
  ret = 0;
//------------------------------------------------------


fun_exit:
  if (fd>0) close(fd);
  return ret;
}



int getUniqueName( char *name )
{
  int ret=0;
#ifdef USE_UUID
  if(name==NULL){
    return -1;
  }

  uuid_t *uuid = (unsigned char*)malloc (sizeof (uuid_t));
  //char *uuid_str = (char*)malloc (37 * sizeof (char));
  uuid_generate_random (uuid);
  uuid_unparse(uuid, name);
  
  //printf ("%s\n", uuid_str);
 
  //free (uuid_str);
  free (uuid);
#endif  
  return ret;
  
}
