//------------------------------------------------------------------------------

#include "Pclient_t.h"
#include "OSFixes.hpp"
#include "flat.h"
#include <pthread.h>

#include <string>
using namespace std;

#define bzero(s, n) memset((s),0,(n))
#define bcopy(s1, s2, n) memmove((s2),(s1),(n))

//------------------------------------------------------------------------------

Pclient_t::Pclient_t()
{
OSFixes::setup_sockets();              // OSfixes: does nothing in Unix, starts
                                       // sockets in Windoze. "Why is this
                                       // necessary?" I hear you ask.
pSkyHook = 0;                          // Callback skyhook back-pointer
pPCB = 0;                              // No packet callback yet
}

//------------------------------------------------------------------------------

Pclient_t::~Pclient_t()
{
}

//------------------------------------------------------------------------------

void Pclient_t::Close()
{
shutdown(sockfd,SH_BOTH);
}

//------------------------------------------------------------------------------

int Pclient_t::Send(vector<byte> _buf)
// Part of main class body: send-side
{
PsMessage(0,"Pclient_t::Send() Just arrived");
char csz[4];
PUT4(&csz[0],_buf.size());
send(sockfd,&csz[0],4,0);              // Send the number of bytes to send
int snt = send(sockfd,&_buf[0],_buf.size(),0);    // And send them....
                                       // The rest is debug fluff
PsMessage(0,"Sent bytes to server...",_buf.size());
//for (unsigned i=0;i<_buf.size();i++) printf("|%x|",_buf[i]);
//printf("\n");  fflush(stdout);
return snt;
}

//------------------------------------------------------------------------------

void Pclient_t::SetPCB(void(* _pPCB)(int,void *,vector<byte>,int))
{
PsMessage(0,"Pclient_t::SetPCB() Just arrived");
pPCB = _pPCB;
}

//------------------------------------------------------------------------------

int Pclient_t::Start(string _host,unsigned _port)
// Spin off single thread to receive from server
{
if (pPCB==0) return PsMessage(1,"[ERROR] Client: No receive handler attached");
PsMessage(0,"Client: Receive handler attached OK");
                                       // Create socket
sockfd = socket(AF_INET,SOCK_STREAM,0);
if (sockfd < 0) return PsMessage(2,"[ERROR] Client: Cannot open socket");
else PsMessage(0,"Client: Socket opened OK");
                                       // What server are we after?
struct hostent * server = gethostbyname(_host.c_str());
if (server == NULL) return PsMessage(3,"[ERROR] Client: Server not found");
PsMessage(0,"Client: Found the server OK");

struct sockaddr_in serv_addr;          // Fuck about for a bit
bzero((char *)&serv_addr,sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
serv_addr.sin_port = htons(_port);
                                       // Blocking call
PsMessage(0,"Pclient_t::Start() About to enter connect()");
fflush(stdout);
if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    return PsMessage(4,"[ERROR] Client: cannot connect");
else PsMessage(0,"Client: Socket connected OK");
PsMessage(0,"Pclient_t::Start() Just left connect(), about to launch Recv()");

pthread_t t;                           // Launch receiving thread
pthread_create(&t,0,Recv,(void *)this);
PsMessage(0,"Pclient_t::Start() Just left Recv()");
return 0;
}

//------------------------------------------------------------------------------

void * Recv(void * param)
{
pthread_detach(pthread_self());        // New orphan thread
Pclient_t * pPclient = (Pclient_t *)param;
int sockfd = pPclient->sockfd;         // Extract socket id from argument
int count;                             // Character counter
const int BUFLEN = 2048;               // Yuk
byte buf[BUFLEN];                      // Yuk
pPclient->PsMessage(0,"Pclient_t ... Recv() About to enter forever loop");
for (;;) {
  pPclient->PsMessage(0,"Pclient_t ... Recv() Inside forever loop\n");
  bzero(buf,BUFLEN);                // Just to make sure
  char csz[4];
  char * psz = &csz[0];
  if (recv(sockfd,psz,4,0)<=0) break;
  uint32 isz = GET4(psz);
  pPclient->PsMessage(0,"Pclient_t ... Recv() expecting bytes ...\n",isz);
  if (isz >= BUFLEN) {
    pPclient->PsMessage(5,"[ERROR] Client: Incoming too large for buffer");
    break;
  }
  count = recv(sockfd,buf,isz,0);  // Get it back?  Blocking read
  if (count < 0) break;
  vector<byte> x_v;
  for(int u=0;u<count;u++)x_v.push_back(buf[u]); // Seriously?
  (pPclient->pPCB)(sockfd,pPclient->pSkyHook,x_v,count);
}
pPclient->PsMessage(0,"Pclient_t ... Recv() About to leave");
shutdown(sockfd,SH_BOTH);
return 0;
}

//==============================================================================
