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
#include <signal.h>
#include <poll.h>
#define MAX_LINE 65536
#define MAX_PENDING 5
static int ctlc_hit = 0;
int sockOK = 0;
void sigpipe_handler() {
  puts("SIGPIPE caught");
  sockOK=0;
}
void ctlC_handler(){
  if(ctlc_hit)
    exit(0);
  ctlc_hit++;
  puts("SIGINT caught - ^C again to exit");
  sockOK = 0;
  return;
}
int main(int argc, char** argv) {

struct sockaddr_in sin;
char *host;
char buf[MAX_LINE];
int s,new_s,len;
int SERVER_PORT;

  if(argc<2){
    printf("Give me a port number matey");
    exit(0);
  }

  SERVER_PORT = atoi(argv[1]);
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
    printf("socket creation error %s\n",strerror(errno));
    exit(0);
  }

  /*
  	bind socket to address
  */
  if(bind(s,(struct sockaddr*)&sin, sizeof(sin))<0) {
    printf("binding error %s",strerror(errno));
    close(s);
    exit(0);
  }
  listen(s,MAX_PENDING);
  /*
  	get some lines
  */
  signal(SIGPIPE,sigpipe_handler);
  //signal(SIGINT,ctlC_handler);
  while(1){
    if((new_s=accept(s,(struct sockaddr *)&sin,&len))<0) {
      printf("acceptance error len = %d - %s\n",len,strerror(errno));
      close(new_s);
      break;
    }
    struct pollfd  pfd = {new_s,POLLIN,0};
    sockOK = 1;
    while(sockOK){
      poll(&pfd,1,5000);
      if(pfd.revents & POLLIN) {
        len=recv(new_s,buf,sizeof(buf),0);
        if(len == 0) /*socket is closed */
          sockOK = 0;
        else
          puts(buf);
      }
    }
    printf("client side appears dead server closing connection\n");
    close(new_s);
  }
}



