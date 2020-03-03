#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void generateData(int * data, int SIZE);

int compfn (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}


//Do not change the seed
#define SEED 72
#define MAXVAL 30

//Total input size is N
//Doesn't matter if N doesn't evenly divide nprocs
#define N 20

int main(int argc, char **argv) {

  int my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  //seed rng do not modify
  srand(SEED+my_rank);

  //local input size N/nprocs
  const unsigned int localN=N/nprocs;

  //All ranks generate data
  int * data=(int*)malloc(sizeof(int)*localN);

  generateData(data, localN);

  int * sendDataSetBuffer=(int*)malloc(sizeof(int)*localN); //most that can be sent is localN elements
  int * recvDatasetBuffer=(int*)malloc(sizeof(int)*localN); //most that can be received is localN elements
  int * myDataSet=(int*)malloc(sizeof(int)*N); //upper bound size is N elements for the rank

  //Write code here

  int globalSum, localSum;
  localSum = 0;
  // calculate the local sum in all ranks
  for (int i = 0; i < localN; i++) {
      localSum += data[i];
  }
  // Send localsum from all ranks to 0 and reduce the sum into global sum
  MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  // Print global sum by rank 0
  if (my_rank == 0){
    printf("#######################################\n");
    printf("Global Sum before sorting: %d", globalSum);
    printf("\n#######################################\n");
  }

  struct range {
    int min;
    int max;
  } *dataRange;

  dataRange = (struct range*) malloc(sizeof(struct range) * nprocs);

  int rangeDistance = MAXVAL / nprocs;
  int rangeMover = 0;

  for(int i = 0; i < nprocs; i++) {
    dataRange[i].min = rangeMover;
    dataRange[i].max = rangeMover + rangeDistance;
    rangeMover += rangeDistance;
  }

  printf("Original Data \n");
  for(int i = 0; i < localN; i++) {
    printf("%d ", data[i]);
  }
  printf("\n");

  int datasetCount = 0;
  int * sendBufferCount = (int*)malloc(sizeof(int)*nprocs);
  for(int i = 0; i < nprocs; i++) {
    sendBufferCount[i] = 0;
    for(int j = 0; j < localN; j++) {
      if(data[j] >= dataRange[i].min && data[j] < dataRange[i].max) {
        if(i == my_rank) {
          myDataSet[datasetCount] = data[j];
          datasetCount++;
        } else {
          sendDataSetBuffer[sendBufferCount[i]] = data[j];
          sendBufferCount[i]++;
        }
      }
    }
    if( i != my_rank) {
      MPI_Send(&sendBufferCount[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      MPI_Send(sendDataSetBuffer, sendBufferCount[i], MPI_INT, i, 0, MPI_COMM_WORLD);
    }
  }

  int * receiveBufferCount = (int*)malloc(sizeof(int)*nprocs);
  for(int i = 0; i < nprocs; i++) {
    if(i != my_rank) {
      MPI_Status status1, status2;
      MPI_Recv(&receiveBufferCount[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status1);
      MPI_Recv(recvDatasetBuffer, receiveBufferCount[i], MPI_INT, i, 0, MPI_COMM_WORLD, &status2);
    }
  }

  for(int i = 0; i < nprocs; i++) {
    if(i != my_rank) {
      for(int j = 0; j < receiveBufferCount[i]; j++) {
        myDataSet[datasetCount] = recvDatasetBuffer[j];
        datasetCount++;
      }
    }
  }

  // Sorting
  qsort(myDataSet, datasetCount, sizeof(int), compfn);

  printf("Rank %d has new data: \n", my_rank);
  for(int i = 0; i < datasetCount; i++) {
    printf("%d ", myDataSet[i]);
  }
  printf("\n");

  localSum = 0;
  // calculate the local sum in all ranks
  for (int i = 0; i < localN; i++) {
      localSum += data[i];
  }
  // Send localsum from all ranks to 0 and reduce the sum into global sum
  MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  // Print global sum by rank 0
  if (my_rank == 0) {
    printf("#######################################\n");
    printf("Global Sum after sorting: %d", globalSum);
    printf("\n#######################################\n");
  }

  MPI_Finalize();
  return 0;
}

//generates data [0,MAXVAL)
void generateData(int * data, int SIZE)
{
  for (int i=0; i<SIZE; i++)
  {
  
  data[i]=rand()%MAXVAL;
  }
}