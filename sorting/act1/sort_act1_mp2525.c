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

  // Declaring time variables
  double t0, t1, t2, t3, distributionTime, sortingTime, totalTime;

  // Declaring global and localsum variables
  long unsigned int globalSum, localSum;

  /******************************************
  * Global Sum Calculation before sorting
  * *****************************************
  */

  // Calculate local sum in a rank
  localSum = 0;
  for (int i = 0; i < localN; i++) {
    localSum += data[i];
  }

  // Send localsum from all ranks to 0 and reduce the sum into global sum
  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
             MPI_COMM_WORLD);

  // Print global sum from rank 0
  if (my_rank == 0) {
    printf("#######################################\n");
    printf("Global Sum before sorting: %lu", globalSum);
    printf("\n#######################################\n");
  }

  // Start the total time calculation
  MPI_Barrier(MPI_COMM_WORLD);
  t0 = MPI_Wtime();

  /******************************************
  * Range calculation and broadcasting
  * *****************************************
  */

  // Allocate memory for data range
  long unsigned int **dataRange =
      (long unsigned int **)malloc(sizeof(long unsigned int *) * nprocs);
  for (int i = 0; i < nprocs; i++) {
    dataRange[i] = (long unsigned int *)malloc(sizeof(long unsigned int) * 2);
  }

  // Calculate data ranges in rank 0
  if (my_rank == 0) {
    // Range distance is the interval between range
    long unsigned int rangeDistance = MAXVAL / nprocs;

    // Range mover is iterated over nprocs to set the range
    long unsigned int rangeMover = 0;

    for (int i = 0; i < nprocs; i++) {
      dataRange[i][0] = rangeMover;
      dataRange[i][1] = i == nprocs - 1 ? MAXVAL : rangeMover + rangeDistance;
      rangeMover += rangeDistance;
    }
  }

  // Broadcast the data range to all the ranks
  for (int i = 0; i < nprocs; i++) {
    MPI_Bcast(dataRange[i], 2, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
  }

  /******************************************
  * Data Distribution
  * *****************************************
  */

  // Start time calculation for bucketing / data distribution process
  MPI_Barrier(MPI_COMM_WORLD);
  t1 = MPI_Wtime();

  // dataset count maintains the size of mydataset for a rank
  long unsigned int datasetCount = 0;

  // Iterate over all the ranks to set mydataset and sending & receiving data.
  for (int i = 0; i < nprocs; i++) {
    // Set mydataset if the data is on its own range
    if (i == my_rank) {
      for (long unsigned int j = 0; j < localN; j++) {
        if (data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) {
          myDataSet[datasetCount] = data[j];
          datasetCount += 1;
        }
      }
    }
    // Else, collect data, send the data and receive from rank i
    else {
      // sendcount and receivecount maintains the size of data being sent and
      // received
      long unsigned int sendCount = 0;
      long unsigned int receiveCount = 0;

      // collect data to be sent in sendDataSetBuffer
      for (long unsigned int j = 0; j < localN; j++) {
        if (data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) {
          sendDataSetBuffer[sendCount] = data[j];
          sendCount += 1;
        }
      }

      // send data in sendDataSetBuffer to rank i
      MPI_Request request1, request2;
      MPI_Status status1, status2;
      MPI_Isend(&sendCount, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD,
                &request1);
      MPI_Isend(sendDataSetBuffer, sendCount, MPI_INT, i, 1, MPI_COMM_WORLD,
                &request2);

      // Receive data from rank i, set into recvDataSetBuffer
      MPI_Status status3, status4;
      MPI_Recv(&receiveCount, 1, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD,
               &status3);
      MPI_Recv(recvDatasetBuffer, receiveCount, MPI_INT, i, 1, MPI_COMM_WORLD,
               &status4);

      // Update myDataSet by adding data from receiveDataSetBuffer
      for (long unsigned int x = 0; x < receiveCount; x++) {
        myDataSet[datasetCount] = recvDatasetBuffer[x];
        datasetCount += 1;
      }

      // Wait for the asyncgronous send
      // Also, avoids the error of rank not been terminated properly
      MPI_Wait(&request1, &status1);
      MPI_Wait(&request2, &status2);
    }
  }

  // End data distribution time and start data sorting time
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

  // Calculate local distribution time
  double localDistributionTime = t2 - t1;

  // Calculate local sorting time
  double localSortingTime = t3 - t2;

  // Calculate local total time
  double localTotalTime = t3 - t0;

  // Send the local distribution time data and reduce into max value in rank 0
  MPI_Reduce(&localDistributionTime, &distributionTime, 1, MPI_DOUBLE, MPI_MAX,
             0, MPI_COMM_WORLD);

  // Send the local sorting time data and reduce into max value in rank 0
  MPI_Reduce(&localSortingTime, &sortingTime, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  // Send the local total time data and reduce into max value in rank 0
  MPI_Reduce(&localTotalTime, &totalTime, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  // Display max time measurements from rank 0
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

  // Calculate local sum in a rank
  localSum = 0;
  for (long unsigned int i = 0; i < datasetCount; i++) {
    localSum += myDataSet[i];
  }

  // Send localsum from all ranks to 0 and reduce the sum into global sum
  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
             MPI_COMM_WORLD);

  // Print global sum from rank 0
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