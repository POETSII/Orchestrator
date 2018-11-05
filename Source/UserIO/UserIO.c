#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "MPI_ports.h"
void server(),client(),responder(),requester();

void main(int argc,char* argv[])
{

int ProcNo,NoOfProcs;
  MPI_Init(&argc,&argv);

  MPI_Comm_size(MPI_COMM_WORLD,&NoOfProcs);
  MPI_Comm_rank(MPI_COMM_WORLD,&ProcNo);


  
  if(ProcNo==UserIO_Server){
    server();
    puts("UserIO has asked for the server to start");
  }

  if(ProcNo==UserIO_Client){
    client();
    printf("UserIO has asked for the client to start");
  }
 
  if(ProcNo==UserIO_Responder){
    responder();
    printf("UserIO has asked for the messenger to start");
  }  

  if(ProcNo==UserIO_Requester){
    requester();
    printf("UserIO has asked for the messenger to start");
  }  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}






