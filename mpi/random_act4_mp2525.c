#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

// Example compilation
// mpicc random_comm_starter.c -lm -o random_comm_starter

// Example execution
// mpirun -np 1 -hostfile myhostfile.txt ./random_comm_starter

// Do not change the seed, or your answer will not be correct
#define SEED 72

// Change this if you want, but make sure it is set to 10 when submitting the assignment
#define TOTALITER 10

int generateRandomRank(int max_rank, int my_rank);

int main(int argc, char **argv) {
  int i, my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  if (nprocs<3)
  {
    if (my_rank==0)
      printf("\nRun with at least 3 ranks.\n\n");
    MPI_Finalize();
    return 0;
  }

  // seed rng do not modify
  srand(SEED+my_rank);


  // WRITE CODE HERE  
  int next_rank;
  int counter;

  next_rank = generateRandomRank(nprocs - 1, my_rank);

  if(my_rank == 0) {
    MPI_Request request;
    MPI_Status status;

    printf("Master: first rank: %d \n", next_rank);
    
    counter = 0;
    MPI_Send(&counter, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);

  } else {
      
      MPI_Status status;
      MPI_Request request;
      
      MPI_Irecv(&counter, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);
      MPI_Wait(&request, &status);

      printf("My rank: %d, old counter: %d \n", my_rank, counter);
      
      counter = counter + my_rank;
      printf("My rank: %d, new counter: %d \n", my_rank, counter);

      next_rank = generateRandomRank(nprocs - 1, my_rank);
      printf("My rank: %d, next to recv: %d \n", my_rank, next_rank);

      
      MPI_Send(&counter, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);

  }   

  MPI_Finalize();
  return 0;
}


// Do not modify the rank generator or you will get the wrong answer
// returns a rank between 1 and max_rank, but does not return itself
// does not generate rank 0
int generateRandomRank(int max_rank, int my_rank)
{
  
  int tmp=round(max_rank*((double)(rand()) / RAND_MAX));
  while(tmp==my_rank || tmp==0)
  {  
  tmp=round(max_rank*((double)(rand()) / RAND_MAX)); 
  }

  return tmp;
}



