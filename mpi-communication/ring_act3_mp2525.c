#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Example compilation
// mpicc ring_comm_starter.c -lm -o ring_comm_starter

// Example execution
// mpirun -np 2 -hostfile myhostfile.txt ./ring_comm_starter

int main(int argc, char **argv) {

  int my_rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (nprocs == 1) {
    printf("\n\nEnter at least 2 process ranks\n\n");
    MPI_Finalize();
    return 0;
  }

 	//Write code here

  int ringlimit = 10;

 	// Counter of a rank
  int ringcount = 0;

  // successor rank of a rank
  int next_rank = (my_rank + 1) % nprocs;

  // Predecessor rannk of a rank
  int prev_rank = my_rank == 0 ? nprocs - 1 : (my_rank - 1) % nprocs;

  for (int i = 0; i < ringlimit; i++) {

    int to_increment;
    MPI_Status status;
    MPI_Request request;

   	// Send message to successor rank
    MPI_Isend(&my_rank, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD, &request);

   	// Receive message from predecessor rank
    MPI_Recv(&to_increment, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &status);

   	// Update the counter
    ringcount += to_increment;

  }

  printf("\n My Rank is %d and ring count is %d \n", my_rank, ringcount);

  MPI_Finalize();
  return 0;
}