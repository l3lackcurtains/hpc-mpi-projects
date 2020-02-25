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

  int ring_limit = 10;

 	// Counter of a rank
  int ring_count = 0;

  // Successor rank in a ring
  int next_rank = (my_rank + 1) % nprocs;

  // Predecessor rank in a ring
  int prev_rank = (my_rank - 1) % nprocs;

 	// 10 times back and forth process
  for (int i = 0; i < ring_limit; i++) {

    int to_increment;
    MPI_Status status;

    if (my_rank == 0) {

     	// Send message to successor rank
      MPI_Send(&my_rank, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);

     	// Receive message from predecessor rank
      prev_rank = nprocs - 1;
      MPI_Recv(&to_increment, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &status);

     	// Update the counter
      ring_count += to_increment;

    } else {

     	// Receive message from predecessor rank
      MPI_Recv(&to_increment, 1, MPI_INT, prev_rank, 0, MPI_COMM_WORLD, &status);

     	// Update the counter
      ring_count += to_increment;

     	// Send message to successor rank
      MPI_Send(&my_rank, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);

    }
  }

  printf("\n My Rank is %d and ring count is %d \n", my_rank, ring_count);

  MPI_Finalize();
  return 0;
}