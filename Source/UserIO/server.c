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
#include <errno.h>
#include "MPI_ports.h"
#include <signal.h>
#include <poll.h>
#define SERVER_PORT 5432
#define MAX_LINE 65536
#define MAX_PENDING 5

/*
void sigpipe_handler() {
  puts("SIGPIPE caught");
  sockOK=0;
}
*/
int server() {
struct pollfd  pfd;
struct sockaddr_in sin;
char *host;
char buf[MAX_LINE];
int s,new_s,len;
int sockOK = 0;
  
  /*
  	build address structure
  */
  len = sizeof(sin);
  bzero((char*)&sin, sizeof(sin));
  sin.sin_family=AF_INET;
  sin.sin_addr.s_addr=INADDR_ANY;
  sin.sin_port=htons(SERVER_PORT);
  
  /*
  	open as passive
  */
  if((s=socket(PF_INET,SOCK_STREAM,0))<0){
    printf("UserIO-server: socket creation error %s\n",strerror(errno));
    MPI_Abort(MPI_COMM_WORLD,-2);
  }
  
  /*
  	bind socket to address
  */
  if(bind(s,(struct sockaddr*)&sin, sizeof(sin))<0) {
    printf("UserIO-server: binding error %s \n",strerror(errno));
    close(s);
    MPI_Abort(MPI_COMM_WORLD,-2);
  }
  listen(s,MAX_PENDING);
  /*
  	get some lines
  */
  //signal(SIGPIPE,sigpipe_handler);
  while(1){
    if((new_s=accept(s,(struct sockaddr *)&sin,&len))<0) {
      printf("UserIO-server: acceptance error %s\n",strerror(errno));
      close(s);
      continue;
    }
    sockOK = 1;
    pfd.fd = s, pfd.events = POLLIN, pfd.revents = 0;
    if(len=recv(new_s,buf,sizeof(buf),0)){
      MPI_Ssend((void*)buf,len,MPI_UNSIGNED_CHAR,UserIO_Client,101,MPI_COMM_WORLD);
      buf[len+1] = '\0';
      printf("UserIO-server sent to client %s\n",buf);
    }
   /*send messages from the remote client to the requester*/
    while(sockOK){
      poll(&pfd,1,5000);
      /*if(pfd.revents & POLLIN) {*/
        len=recv(new_s,buf,sizeof(buf),0);
        if(len <= 0){ /*socket is closed*/
          sockOK = 0;
          printf("len = %d, socket is broken",len);
          continue;
        }
        MPI_Ssend((void*)buf,len,MPI_UNSIGNED_CHAR,UserIO_Requester,107,MPI_COMM_WORLD);
        buf[len+1] = '\0';
        printf("UserIO-server sent  to Requester %s\n",buf);
     
    }
    close(new_s);  
  }
}    	
	  		
	
  
