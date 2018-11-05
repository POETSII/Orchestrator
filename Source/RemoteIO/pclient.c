#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define MAX_LINE 65536
static int ctlc_hit = 0;
int sockOK = 0;
void sigpipe_handler() {
  puts("SIGPIPE caught");
  sockOK=0;
  return;
}
void ctlC_handler(){
  if(ctlc_hit)
    exit(0);
  ctlc_hit++;
  puts("SIGINT caught - ^C again to exit");
  sockOK = 0;
  return;
}
int main(int argc, char *argv[]) {

struct hostent *hp;
struct sockaddr_in sin;
int SERVER_PORT;
char *host;
char buf[MAX_LINE];
int s,len,err;

  if(argc<3){
    printf("Give me a host name and port number matey");
    exit(0);
  }
  host=argv[1];
  SERVER_PORT = atoi(argv[2]);
  if(!(hp=gethostbyname(host))){	/*get IP address of the host	*/
    printf("%s is unknown",host);
    exit(0);
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
    printf("socket creation error %s\n",strerror(errno));
    close(s);
    return -1;
  }
  
  /*
  	connect socket to address
  */
  if(connect(s,(struct sockaddr*)&sin, sizeof(sin))<0) {
    printf("socket connection error %s\n",strerror(errno));
    close(s);
    return -1;
  }
  /*
  	send some lines
  */
  signal(SIGPIPE,sigpipe_handler);
  //signal(SIGINT,ctlC_handler);
  sockOK = 1;
  struct pollfd  pfd = {s,POLLOUT,0};
  while(sockOK && fgets(buf,sizeof(buf),stdin)){
    buf[MAX_LINE-1]='\0';
    len=strlen(buf)+1;	
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
        printf("%d bytes sent\n",err);
    }
    else
      sockOK = 0;
  }
  printf("server closing client socket - pipe broken it seems\n");
  close(s);
  return 0;
}    	
	  		
	
  
