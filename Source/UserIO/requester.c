#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MPI_ports.h"
#define MAX_LINE 256
void requester() {
  char buf[MAX_LINE];
  int len;
  MPI_Status Rstatus;
  /*
  	send some lines
  */

  while(1){
    MPI_Recv((void*)buf,MAX_LINE,MPI_UNSIGNED_CHAR,UserIO_Server,107,MPI_COMM_WORLD,&Rstatus);
    MPI_Get_count(&Rstatus,MPI_UNSIGNED_CHAR,&len);
    buf[len+1] = '\0';
    printf("UserIO_requester: about to send %s\n",buf);
    printf("UserIO_requester: entering synchronous send\n");
    MPI_Ssend((void*)buf,len,MPI_UNSIGNED_CHAR,UserIO_Responder,109,MPI_COMM_WORLD);
  }
}



