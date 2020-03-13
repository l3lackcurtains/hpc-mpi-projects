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
  int *myDataSet = (int *)malloc(
      sizeof(int) * N);  // upper bound size is N elements for the rank

  // Write code here

  double t0, t1, t2, t3, distributionTime, sortingTime, totalTime;
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
  * Generating Histogram, Range Calculation
  * and broadcasting
  * *****************************************
  */

  // Global Data Counter maintains the frequency of data in all ranks
  unsigned int *globalDataCounter =
      (unsigned int *)malloc(sizeof(unsigned int) * MAXVAL);

  // Data Counter maintains the frequency of data in a rank
  unsigned int *dataCounter =
      (unsigned int *)malloc(sizeof(unsigned int) * MAXVAL);

  // Allocate memory for data range
  unsigned int **dataRange =
      (unsigned int **)malloc(sizeof(unsigned int *) * nprocs);
  for (int i = 0; i < nprocs; i++) {
    dataRange[i] = (unsigned int *)malloc(sizeof(unsigned int) * 2);
  }

  // Initialize datacounter as 0
  for (int i = 0; i < MAXVAL; i++) {
    dataCounter[i] = 0;
    globalDataCounter[i] = 0;
  }

  // Rank computes frequency of data in dataCounter
  for (int i = 0; i < localN; i++) {
    dataCounter[data[i]] += 1;
  }

  // Send the dataCounter to rank 0 for global data counter calculation
  if (my_rank != 0) {
    MPI_Send(dataCounter, MAXVAL, MPI_UNSIGNED, 0, 2, MPI_COMM_WORLD);
  }

  // Rank 0 receives data counter from other ranks and calculate global data
  // counter
  if (my_rank == 0) {
    for (int i = 0; i < nprocs; i++) {
      // If the rank is not 0, receive dataCounter and update it
      if (i != my_rank) {
        MPI_Status status;
        MPI_Recv(dataCounter, MAXVAL, MPI_UNSIGNED, i, 2, MPI_COMM_WORLD,
                 &status);
      }

      // for all ranks, set glocaldatacounter from datacounter
      for (int i = 0; i < MAXVAL; i++) {
        globalDataCounter[i] += dataCounter[i];
      }
    }

    // Set the percentage of data distribution in each rank
    unsigned int distributionPercentage = N / nprocs;

    // Distribution Range mover to set range for each ranks in a loop
    unsigned int distributionRangeMover = distributionPercentage;

    // Datacounter count data until maxval in a loop
    unsigned int dataCounter = 0;

    // Range min is the minimum value of range
    unsigned int rangeMin = 0;

    // Rank counter is to set range for each rank incrementally
    unsigned int rankCounter = 0;

    for (int j = 0; j < MAXVAL && rankCounter < nprocs; j++) {
      // Datacounter is updated with the frequency value in flobal data counter
      dataCounter += globalDataCounter[j];

      // Set the range, if the datacounter exceeds the distribution range mover
      if (dataCounter >= distributionRangeMover && rankCounter != nprocs - 1) {
        dataRange[rankCounter][0] = rangeMin;
        dataRange[rankCounter][1] = j;
        rangeMin = j;
        distributionRangeMover += distributionPercentage;
        rankCounter++;
      }
      // For last rank, adjust rank to accomodate leftover data
      else {
        dataRange[rankCounter][0] = rangeMin;
        dataRange[rankCounter][1] = MAXVAL;
      }
    }
  }

  // Broadcast the data range to all the ranks
  for (int i = 0; i < nprocs; i++) {
    MPI_Bcast(dataRange[i], 2, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
  }

  /******************************************
  * Data Distribution
  * *****************************************
  */

  // Start time calculation for bucketing / data distribution process
  MPI_Barrier(MPI_COMM_WORLD);
  t1 = MPI_Wtime();

  // dataset count maintains the size of mydataset for a rank
  unsigned int datasetCount = 0;

  // Iterate over all the ranks to set mydataset and sending & receiving data.
  for (int i = 0; i < nprocs; i++) {
    // Set mydataset if the data is on its own range
    if (i == my_rank) {
      for (int j = 0; j < localN; j++) {
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
      unsigned int sendCount = 0;
      unsigned int receiveCount = 0;

      // collect data to be sent in sendDataSetBuffer
      for (int j = 0; j < localN; j++) {
        if (data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) {
          sendDataSetBuffer[sendCount] = data[j];
          sendCount += 1;
        }
      }

      // send data in sendDataSetBuffer to rank i
      MPI_Request request1, request2;
      MPI_Status status1, status2;
      MPI_Isend(&sendCount, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD,
                &request1);
      MPI_Isend(sendDataSetBuffer, sendCount, MPI_INT, i, 1, MPI_COMM_WORLD,
                &request2);

      // Receive data from rank i, set into recvDataSetBuffer
      MPI_Status status3, status4;
      MPI_Recv(&receiveCount, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD,
               &status3);
      MPI_Recv(recvDatasetBuffer, receiveCount, MPI_INT, i, 1, MPI_COMM_WORLD,
               &status4);

      // Update myDataSet by adding data from receiveDataSetBuffer
      for (int x = 0; x < receiveCount; x++) {
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

  qsort(myDataSet, datasetCount, sizeof(int), compfn);

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
  for (int i = 0; i < datasetCount; i++) {
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
  free(globalDataCounter);
  free(dataCounter);

  MPI_Finalize();
  return 0;
}

double randomExponential(double lambda) {
  double u = rand() / (RAND_MAX + 1.0);
  return -log(1 - u) / lambda;
}

// generates data [0,MAXVAL)
void generateData(int *data, int SIZE) {
  for (int i = 0; i < SIZE; i++) {
    double tmp = 0;

    // generate value between 0-1 using exponential distribution
    do {
      tmp = randomExponential(4.0);
      // printf("\nrnd: %f",tmp);
    } while (tmp >= 1.0);

    data[i] = tmp * MAXVAL;
  }
}