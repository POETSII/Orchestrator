#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MPI_ports.h"
#define MAX_LINE 256
void responder() {
  int len;
  char buf[MAX_LINE];
  MPI_Status Rstatus;
   while(1){
    MPI_Recv((void*)buf,MAX_LINE,MPI_UNSIGNED_CHAR,UserIO_Requester,109,MPI_COMM_WORLD,&Rstatus);
    MPI_Get_count(&Rstatus,MPI_UNSIGNED_CHAR,&len);
    buf[len+1] = '\0';
    printf("UserIO_responder: about to send %s\n",buf);
    printf("UserIO_requester: entering synchronous send\n");
    MPI_Ssend((void*)buf,len,MPI_UNSIGNED_CHAR,UserIO_Client,99,MPI_COMM_WORLD);
  }
}    	
	  		
	
  
