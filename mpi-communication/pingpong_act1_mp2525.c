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

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

 	// make sure even number of procs
  if ((nprocs % 2) != 0) {
    if (my_rank == 0) {
      printf("\nYou must enter an even number of process ranks \n");
    }
    MPI_Finalize();
    return 0;
  }

 	// Write code here

  int ping_pong_limit = 5;

 	// Counter of a rank
  int ping_pong_count = 0;

 	// 5 times back and forth process
  for (int i = 0; i < ping_pong_limit; i++) {

    int to_increment;
    MPI_Status status;

    if (my_rank % 2 == 0) {

      // Send it's rank to successor rank
      MPI_Send(&my_rank, 1, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD);

      // Receive incremental value from successor rank
      MPI_Recv(&to_increment, 1, MPI_INT, my_rank + 1, 0, MPI_COMM_WORLD, &status);

     	// Update the counter
      ping_pong_count += to_increment;
      
    } else {

      // Receive incremental value from predecessor rank
      MPI_Recv(&to_increment, 1, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD, &status);

     	// Update the counter
      ping_pong_count += to_increment;

      // Send it's rank to predecessor rank
      MPI_Send(&my_rank, 1, MPI_INT, my_rank - 1, 0, MPI_COMM_WORLD);

    }
  }

  printf("My Rank is %d and ping pong count is %d \n", my_rank, ping_pong_count);

  MPI_Finalize();
  return 0;
}
