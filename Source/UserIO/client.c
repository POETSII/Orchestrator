#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include "MPI_ports.h"
#include <signal.h>
#include <poll.h>
#include <errno.h>
#define MAX_LINE 65536

/*
void sigpipe_handler() {
  puts("SIGPIPE caught");
  sockOK=0;
  return;
}
*/
void client() {
MPI_Status Rstatus;
char* host;
char* port;
struct hostent *hp;
struct sockaddr_in sin;
int sockOK = 0;
char buf[MAX_LINE];
int s,len,slen,i = 0,err;
  int SERVER_PORT;
  int SOURCE;
  struct pollfd  pfd;
  while(1) {
      MPI_Recv((void*)buf,MAX_LINE,MPI_UNSIGNED_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&Rstatus);
      /*SOURCE = MPI_Get_source(&Rstatus);*/
      SOURCE = Rstatus.MPI_SOURCE;
      if(SOURCE == UserIO_Server) {
        printf("UserIO-client: recieved from UserIO-server %s\n",buf);
        strtok(buf, "\n");
        while(buf[i] != ' ')
          i++;
        buf[i] = '\0';
        host = buf;
        i++;
        while(buf[i] == ' ')
          i++;
        port = &buf[i];
        SERVER_PORT = atoi(port);
        printf("UserIO-client: external server is on %s at port %d (in text %s)\n",host,SERVER_PORT,port);
        if(!(hp=gethostbyname(buf))){
          printf("UserIO-client: %s is unknown\n",buf);
          continue;
        }

  /*
  	build address structure
  */
        bzero((char*)&sin, sizeof(sin));
        sin.sin_family=AF_INET;
        bcopy(hp->h_addr,(char*)&sin.sin_addr,hp->h_length);
        sin.sin_port=htons(SERVER_PORT);

  /*
  	open as active
  */
        if((s=socket(PF_INET,SOCK_STREAM,0))<0){
          printf("UserIO-client: socket creation error: %s",strerror(errno));
          continue;
        }

  /*
  	connect socket to address
  */
        if(connect(s,(struct sockaddr*)&sin, sizeof(sin))<0) {
          printf("UserIO-client: socket connection error: %s",strerror(errno));
          close(s);
          continue;
        }
        printf("UserIO-client: is connected to %s at port %d\n",buf,SERVER_PORT);

        //signal(SIGPIPE,sigpipe_handler);
        sockOK = 1;
        pfd.fd = s, pfd.events = POLLOUT, pfd.revents = 0;
/*
  test the connection by sending something back
*/
         char* test = "hello from mpi client";
         len = strlen(test) + 1;
         send(s,test,len,0);
      }
      if(SOURCE == UserIO_Responder)  {
  /*wait for a message from the responder then send it on to the external server*/

        MPI_Get_count(&Rstatus,MPI_UNSIGNED_CHAR,&len);
        buf[len+1] = '\0';
        printf("UserIO-client: recieved %s from UserIO-responder, which is of length %d\n",buf,len);
        if(!sockOK) {
          continue; /*ignore for now but must signal remote and close down self */
        }
        poll(&pfd,1,5000);
        if(pfd.revents & POLLOUT){
          err = send(s,buf,len,0);
          if (err < 0) {
            printf("socket send error %s\n",strerror(errno));
            sockOK = 0;
          }
          else if (err == 0) {
            printf("socket sent nothing assuming pipe broken\n");
            sockOK = 0;
          }
          else
            printf("UserIO-client: sent %s  remote server, which is of length %d\n",buf,len);
        }
        else
          sockOK = 0;
     }
  }
}



