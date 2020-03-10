#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void generateData(int *data, int SIZE);

int compfn(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

// Do not change the seed
#define SEED 72
#define MAXVAL 1000000

// Total input size is N
// Doesn't matter if N doesn't evenly divide nprocs
#define N 1000000000

int main(int argc, char **argv) {
  int my_rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  // seed rng do not modify
  srand(SEED + my_rank);

  // local input size N/nprocs
  const unsigned int localN = N / nprocs;

  // All ranks generate data
  int *data = (int *)malloc(sizeof(int) * localN);

  generateData(data, localN);

  int *sendDataSetBuffer = (int *)malloc(
      sizeof(int) * localN);  // most that can be sent is localN elements
  int *recvDatasetBuffer = (int *)malloc(
      sizeof(int) * localN);  // most that can be received is localN elements

  int *myDataSet = (int *)malloc(sizeof(int) * N);

  // Write code here

  double t0, t1, t2, t3, distributionTime, sortingTime, totalTime;
  long unsigned int globalSum, localSum;

  /******************************************
  * Global Sum Calculation before sorting
  * *****************************************
  */
 

  localSum = 0;
  for (int i = 0; i < localN; i++) {
    localSum += data[i];
  }

  // Send localsum from all ranks to 0 and reduce the sum into global sum

  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  // Print global sum by rank 0
  if (my_rank == 0) {
    printf("#######################################\n");
    printf("Global Sum before sorting: %lu", globalSum);
    printf("\n#######################################\n");
  }
  

  /******************************************
  * Range calculation and broadcasting
  * *****************************************
  */

  // Start data distribution time
  t0 = MPI_Wtime();

  // Data Range memory allocation
  long unsigned int **dataRange = (long unsigned int **)malloc(sizeof(long unsigned int *) * nprocs);
  for (int i = 0; i < nprocs; i++) {
    dataRange[i] = (long unsigned int *)malloc(sizeof(long unsigned int) * 2);
  }

  // Calculate data ranges in rank 0
  long unsigned int rangeDistance = MAXVAL / nprocs;
  long unsigned int rangeMover = 0;
  for (int i = 0; i < nprocs; i++) {
    dataRange[i][0] = rangeMover;
    dataRange[i][1] = i == nprocs - 1 ? MAXVAL : rangeMover + rangeDistance;
    rangeMover += rangeDistance;
  }
  

  /******************************************
  * Data Distribution
  * *****************************************
  */
  MPI_Barrier(MPI_COMM_WORLD);
  t1 = MPI_Wtime();

  long unsigned int datasetCount = 0;
  for(int i = 0; i < nprocs; i++) {
    if(i == my_rank) {
      for(long unsigned int j = 0; j < localN; j++) {
        if(data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) {
          myDataSet[datasetCount] = data[j];
          datasetCount += 1;
        }
      }
    } else {
      long unsigned int sendCount = 0;
      long unsigned int receiveCount = 0;
      for(long unsigned int j = 0; j < localN; j++) {
        if(data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) {
          sendDataSetBuffer[sendCount] = data[j];
          sendCount += 1;
        }
      }
      
      // Send data
      MPI_Request request1, request2;
      MPI_Status status1, status2;
      MPI_Isend(&sendCount, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, &request1);
      MPI_Isend(sendDataSetBuffer, sendCount, MPI_INT, i, 1, MPI_COMM_WORLD, &request2);

      // Receive data
      MPI_Status status3, status4;
      MPI_Recv(&receiveCount, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, &status3);
      MPI_Recv(recvDatasetBuffer, receiveCount, MPI_INT, i, 1, MPI_COMM_WORLD, &status4);
      for(long unsigned int x = 0; x < receiveCount; x++) {
        myDataSet[datasetCount] = recvDatasetBuffer[x];
        datasetCount += 1;
      }

      MPI_Wait(&request1, &status1);
      MPI_Wait(&request2, &status2);
    }
  }

 // End data distribution time
  MPI_Barrier(MPI_COMM_WORLD);
  t2 = MPI_Wtime();

  /******************************************
  * Sorting
  * *****************************************
  */

  qsort(myDataSet, datasetCount, sizeof(myDataSet[0]), compfn);

  // End Sorting time
  MPI_Barrier(MPI_COMM_WORLD);
  t3 = MPI_Wtime();

  /******************************************
  * Global time calculation
  * *****************************************
  */

  double localDistributionTime = t2 - t1;
  double localSortingTime = t3 - t2;
  double localTotalTime = t3 - t0;

  MPI_Reduce(&localDistributionTime, &distributionTime, 1, MPI_DOUBLE, MPI_MAX,
             0, MPI_COMM_WORLD);

  MPI_Reduce(&localSortingTime, &sortingTime, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  MPI_Reduce(&localTotalTime, &totalTime, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  if (my_rank == 0) {
    printf("#######################################\n");
    printf("Time to distribute data: %f\n", distributionTime);
    printf("Time to Sort the data: %f\n", sortingTime);
    printf("TOTAL time taken: %f", totalTime);
    printf("\n#######################################\n");
  }

  /******************************************
  * Global Sum Calculation after sorting
  * *****************************************
  */

  
  localSum = 0;
  for (long unsigned int i = 0; i < datasetCount; i++) {
    localSum += myDataSet[i];
  }
  

  // Send localsum from all ranks to 0 and reduce the sum into global sum
  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  // Print global sum by rank 0
  if (my_rank == 0) {
    printf("#######################################\n");
    printf("Global Sum after sorting: %lu", globalSum);
    printf("\n#######################################\n");
  }

  /******************************************
  * Free memory allocation
  * *****************************************
  */
  free(data);
  free(sendDataSetBuffer);
  free(recvDatasetBuffer);
  free(myDataSet);
  for (int i = 0; i < nprocs; i++) {
    free(dataRange[i]);
  }
  free(dataRange);

  MPI_Finalize();
  return 0;
}

// generates data [0,MAXVAL)
void generateData(int *data, int SIZE) {
  for (int i = 0; i < SIZE; i++) {
    data[i] = rand() % MAXVAL;
  }
}