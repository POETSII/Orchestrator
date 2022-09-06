//------------------------------------------------------------------------------

#include <stdio.h>
#include "Pserver_t.h"
#include "OSFixes.hpp"
#include "flat.h"
#include <pthread.h>

#define bzero(s,n) memset((s),0,(n))

//==============================================================================

Pserver_t::Pserver_t()
{
printf("********* Pserver_t constructor\n");
fflush(stdout);

OSFixes::setup_sockets();              // OSfixes: does nothing in Unix, starts
                                       // sockets in Windoze. "Why is this
                                       // necessary?" I hear you ask.
pSkyHook = 0;                          // Callback skyhook back-pointer
SetMCB(0);
SetPCB(0);
}

//------------------------------------------------------------------------------

Pserver_t::~Pserver_t()
{
printf("********* Pserver_t destructor\n");
}

//------------------------------------------------------------------------------

void Pserver_t::Close()
{
OSFixes::close_socket(sockfd);
}

//------------------------------------------------------------------------------

int Pserver_t::Recv(unsigned _port)
// Listen on a fixed input port
{
                                       // If there's no incoming packet handler
                                       // defined, we're just wasting time
if (pPCB==DefPCB) return PsMessage(10,"[ERROR] Server: no packet callback defined");
                                       // Create a socket
sockfd = socket(AF_INET,SOCK_STREAM,0);

//ioctlsocket(sockfd,FIONBIO,0);
// None of this seems to work....
//int optval = 1;
//if ((setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int))) == -1) ;
//setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const char *)&optval,sizeof(int)) ;
//int flags = fcntl(sockfd,F_GETFL,0);   /* get socket's flags */
//flags |= O_NONBLOCK;  /* Add O_NONBLOCK status to socket descriptor's flags */
//status = fcntl(sockfd, F_SETFL, flags); /* Apply the new flags to the socket */

if (sockfd < 0) return PsMessage(11,"[ERROR] Server: cannot open socket");
struct sockaddr_in serv_addr;          // Clear address structure
bzero((char *)&serv_addr,sizeof(serv_addr));
                                       // Set up host_addr for use in bind call
                                       // Server byte order (sigh)
serv_addr.sin_family = AF_INET;
                                       // Automatically filled with current
                                       // host IP address
serv_addr.sin_addr.s_addr = INADDR_ANY;
                                       // Convert short integer port to
                                       // network byte order (sigh)
serv_addr.sin_port = htons(_port);

// bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
// bind() passes file descriptor, address structure, length of address structure
// The call will bind the socket to the current IP address on port _port
// Note the implied union: serv_addr is of type sockaddr_in, not sockaddr
int resp = bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
if (resp < 0) return PsMessage(12,"[ERROR] SERVER: Cannot bind socket");

// listen() call tells the socket to listen to the incoming connections.
// The function places all incoming connections into a backlog queue until
// accept() accepts the connection
// Maximum backlog queue length = BQL: I don't know what this atually does/means
const int BQL = 5;
listen(sockfd,BQL);

pthread_t t;                           // Launch receiving thread: this is one
                                       // thread per connection
pthread_create(&t,0,Recv0,(void *)this);
PsMessage(0,"Pserver_t::Recv() Just returned from thread create Recv0()");
return 0;
}

//------------------------------------------------------------------------------

int Pserver_t::Send(int _socket,vector<byte> _buf)
// This does all the heavy lifting for the send activities:
{
PsMessage(0,"Pserver_t::Send() Just arrived");
uchar csz[4];
PUT4(&csz[0],_buf.size());             // Number of bytes to send
// Debug...
uint32 isz=GET4(&csz[0]);
printf("Pserver_t::Send isz = %d\n",isz);
printf("_buf.size() = %lu\n",_buf.size());
fflush(stdout);
//-------------
send(_socket,&csz[0],4,0);              // Send the number of bytes to send
return send(_socket,&_buf[0],_buf.size(),0);     // And send them....
}

//------------------------------------------------------------------------------

void Pserver_t::SetPCB(void(* _pPCB)(int,void *,vector<byte>,int))
// Connect the third party packet handler
{
PsMessage(0,"Pserver_t::SetPCB() Just arrived");
pPCB = (_pPCB==0) ? DefPCB : _pPCB;    // If null, use the default
}


//==============================================================================

void DefPCB(int socket,void * pSkyHook,vector<byte>buf,int count)
// Default packet handler - should be disconnected and overridden by the user
{
printf("\nPserver_t default message handler....\n");
printf("SkyHook: %" PTR_FMT "\n",reinterpret_cast<uint64_t>(pSkyHook));
printf("Message (%d bytes): ---[\n",count);
for(unsigned i=0;i<buf.size();i++) {
  if (i%80 == 0) printf("\n");
  printf("%c",buf[i]);
}
printf("]---\n\n");
fflush(stdout);
}

//------------------------------------------------------------------------------

void * Recv0(void * param)
// Thread function spun off from Pserver_t::listen() to handle a connection.
// The reason for this is to allow the main thread to go about its business of
// asynchronously sending to the client.
{
pthread_detach(pthread_self());
Pserver_t * pPserver = (Pserver_t *)param;
pPserver->PsMessage(0,"Pserver_t::Recv0() About to enter accept() loop");
                                       // Accepts incoming connection (blocking)
while((pPserver->newsockfd=accept(pPserver->sockfd,0,0))>0) {
  pPserver->PsMessage(0,"Pserver_t::Recv0() Inside accept() loop");
  fflush(stdout);
  pthread_t t;                        // Spin off unique thread for this
  pthread_create(&t,0,Recv1,param);   // connection
}
pPserver->PsMessage(0,"Pserver_t::Recv0() Leaving slave thread: - outside accept() loop");
//shutdown(pPserver->newsockfd,SH_BOTH);
return 0;
}

//------------------------------------------------------------------------------

void * Recv1(void * param)
// Thread function - one of many. Each one talks to a different client.
{
pthread_detach(pthread_self());        // New orphan thread
Pserver_t * pPserver = (Pserver_t *)param;
int newsockfd = pPserver->newsockfd;   // Extract socket id from argument
int count;                             // Character counter
const int BUFLEN = 2048;               // Yuk
byte buf[BUFLEN];                      // Yuk
bzero(buf,BUFLEN);
pPserver->PsMessage(0,"Pserver_t ... Recv1() About to enter (connected) loop");
while (true) {                         // Assume we're connected until broken
  uchar csz[4];                         // Size of the message to follow
  uchar * psz = &csz[0];
  if (recv(newsockfd,psz,4,0)<=0) break;   // Socket closed?
  int isz = GET4(psz);                 // Incoming message size as an integer
  pPserver->PsMessage(0,"Pserver_t ... Recv1() expecting bytes...",isz);
  fflush(stdout);
  if (isz >= BUFLEN) {
    (pPserver->pMCB)(0,"Incoming message too large for buffer");
    break;
  }
  if((count = recv(newsockfd,buf,isz,0))<=0) break;    // Blocking call
//-------------  Debug stuff
  pPserver->PsMessage(0,"Pserver_t ... Recv1() Inside recv() loop");
  printf("Pserver_t ... Just pulled in %d bytes\n",count);
  for(int i=0;i<count;i++) printf("|%x|",buf[i]);
  printf("\n");
  fflush(stdout);
//-------------
  vector<byte> x_v;
  for(int u=0;u<count;u++)x_v.push_back(buf[u]); // Seriously?
  bzero(buf,BUFLEN);
  (pPserver->pPCB)(newsockfd,pPserver->pSkyHook,x_v,count);
}
pPserver->PsMessage(0,"Pserver_t ... Recv1() About to leave");
shutdown(newsockfd,SH_BOTH);           // Close the socket for this thread
return 0;
}

//------------------------------------------------------------------------------
