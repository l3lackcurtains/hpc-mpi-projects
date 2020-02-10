#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Example compilation
// mpicc comm_pingpong_starter.c -lm -o comm_pingpong_starter

// Example execution
// mpirun -np 2 -hostfile myhostfile.txt ./comm_pingpong_starter

int main(int argc, char **argv) {

  int my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  // make sure even number of procs
  if ((nprocs%2)!=0)
  {
    if (my_rank==0)  
    {
    printf("\nYou must enter an even number of process ranks \n"); 
    }
    MPI_Finalize();
    return 0;
  }

  //Write code here
  printf("Hello from %d \n", my_rank);



  MPI_Finalize();
  return 0;
}