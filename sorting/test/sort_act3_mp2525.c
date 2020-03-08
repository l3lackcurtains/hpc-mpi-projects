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
#define MAXVAL 100

// Total input size is N
// Doesn't matter if N doesn't evenly divide nprocs
#define N 1000

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

  printf("Rank %d has random data: ", my_rank);
  for(int i = 0; i < localN; i++) {
    printf("%d ", data[i]);
  }
  printf("\n");

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
  * Generating Histogram, Range Calculation
  * and broadcasting
  * *****************************************
  */

  // Start data distribution time
  MPI_Barrier(MPI_COMM_WORLD);
  t0 = MPI_Wtime();

  int *globalDataCounter = (int *)malloc(sizeof(int) * MAXVAL);
  int *dataCounter = (int *)malloc(sizeof(int) * MAXVAL);

  int **dataRange = (int **)malloc(sizeof(int *) * nprocs);
  for (int i = 0; i < nprocs; i++) {
    dataRange[i] = (int *)malloc(sizeof(int) * 2);
  }

  // Initialize datacounter as 0
  for (int i = 0; i < MAXVAL; i++) {
    dataCounter[i] = 0;
    globalDataCounter[i] = 0;
  }

  for (int i = 0; i < localN; i++) {
    dataCounter[data[i]] += 1;
  }

  if (my_rank != 0) {
    MPI_Send(dataCounter, MAXVAL, MPI_INT, 0, 2, MPI_COMM_WORLD);
  }

  if (my_rank == 0) {
    for (int i = 0; i < nprocs; i++) {
      if (i != my_rank) {
        MPI_Status status;
        MPI_Recv(dataCounter, MAXVAL, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
      }
      for (int i = 0; i < MAXVAL; i++) {
        globalDataCounter[i] += dataCounter[i];
      }
    }

    int distributionPercentage = N / nprocs;
    int distributionRangeMover = distributionPercentage;
    int dataCounter = 0;
    int rangeMin = 0;
    int procCounter = 0;
    for (int j = 0; j < MAXVAL && procCounter < nprocs; j++) {
      dataCounter += globalDataCounter[j];
      if (dataCounter >= distributionRangeMover && procCounter != nprocs - 1) {
        dataRange[procCounter][0] = rangeMin;
        dataRange[procCounter][1] = j + 1;
        rangeMin = j + 1;
        distributionRangeMover += distributionPercentage;
        procCounter++;
      } else {
        dataRange[procCounter][0] = rangeMin;
        dataRange[procCounter][1] = MAXVAL;
      }
    }
  }

  for (int i = 0; i < nprocs; i++) {
    MPI_Bcast(dataRange[i], 2, MPI_INT, 0, MPI_COMM_WORLD);
  }

  if(my_rank == 0) {
    for(int i = 0; i < nprocs; i++) {
      printf("Rank %d has data range [%d, %d)\n", i, dataRange[i][0], dataRange[i][1]);
    }
    printf("======================================\n");
  }

  /******************************************
  * Data Distribution
  * *****************************************
  */

  // Start data distribution time
  MPI_Barrier(MPI_COMM_WORLD);
  t1 = MPI_Wtime();

  // Send buffer data to other ranks
  int datasetCount = 0;
  int *sendBufferCount = (int *)malloc(sizeof(int) * nprocs);
  for (int i = 0; i < nprocs; i++) {
    sendBufferCount[i] = 0;
    for (int j = 0; j < localN; j++) {
      if (data[j] >= dataRange[i][0] && data[j] < dataRange[i][1]) {
        if (i == my_rank) {
          myDataSet[datasetCount] = data[j];
          datasetCount++;
        } else {
          sendDataSetBuffer[sendBufferCount[i]] = data[j];
          sendBufferCount[i]++;
        }
      }
    }
    if (i != my_rank) {
      MPI_Send(&sendBufferCount[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      MPI_Send(sendDataSetBuffer, sendBufferCount[i], MPI_INT, i, 0,
               MPI_COMM_WORLD);
    }
  }

  // Receive buffer data to other ranks
  int *receiveBufferCount = (int *)malloc(sizeof(int) * nprocs);
  for (int i = 0; i < nprocs; i++) {
    if (i != my_rank) {
      MPI_Status status1, status2;
      MPI_Recv(&receiveBufferCount[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD,
               &status1);
      MPI_Recv(recvDatasetBuffer, receiveBufferCount[i], MPI_INT, i, 0,
               MPI_COMM_WORLD, &status2);
      for (int j = 0; j < receiveBufferCount[i]; j++) {
        myDataSet[datasetCount] = recvDatasetBuffer[j];
        datasetCount++;
      }
    }
  }

  printf("Rank %d has dataset: ", my_rank);
  for(int i = 0; i < datasetCount; i++) {
    printf("%d ", myDataSet[i]);
  }
  printf("\n");

  // End data distribution time
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

  printf("After sorting rank %d has dataset: ", my_rank);
  for(int i = 0; i < datasetCount; i++) {
    printf("%d ", myDataSet[i]);
  }
  printf("\n");

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
  for (int i = 0; i < datasetCount; i++) {
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
  free(sendBufferCount);
  free(receiveBufferCount);

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