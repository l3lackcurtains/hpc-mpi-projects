#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "RTree.h"

// Example compilation
// mpic++ -O3 range_query_rtree_starter.cpp -lm -o range_query_rtree_starter

// Example execution
// mpirun -np 1 -hostfile myhostfile.txt ./range_query_rtree_starter 1000000
// 1000

struct dataStruct {
  double x;
  double y;
};

struct queryStruct {
  double x_min;
  double y_min;
  double x_max;
  double y_max;
};

///////////////////////
// For R-tree

bool MySearchCallback(int id, void *arg) {
  // printf("Hit data rect %d\n", id);
  return true;  // keep going
}

struct Rect {
  Rect() {}

  Rect(double a_minX, double a_minY, double a_maxX, double a_maxY) {
    min[0] = a_minX;
    min[1] = a_minY;

    max[0] = a_maxX;
    max[1] = a_maxY;
  }

  double min[2];
  double max[2];
};

///////////////////////

void generateData(struct dataStruct *data, unsigned int localN);
void generateQueries(struct queryStruct *data, unsigned int localQ,
                     int my_rank);

// Do not change constants
#define SEED 72
#define MAXVAL 100.0
#define QUERYRNG 10.0

int main(int argc, char **argv) {
  int my_rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  // Process command-line arguments
  int N;
  int Q;

  if (argc != 3) {
    fprintf(stderr,
            "Please provide the following on the command line: <Num data "
            "points> <Num query points>\n");
    MPI_Finalize();
    exit(0);
  }

  sscanf(argv[1], "%d", &N);
  sscanf(argv[2], "%d", &Q);

  const unsigned int localN = N;
  const unsigned int localQ = Q / nprocs;

  // local storage for the number of results of each query -- init to 0
  unsigned int *numResults =
      (unsigned int *)calloc(localQ, sizeof(unsigned int));

  // All ranks generate the same input data
  struct dataStruct *data =
      (struct dataStruct *)malloc(sizeof(struct dataStruct) * localN);
  generateData(data, localN);

  // All ranks generate different queries
  struct queryStruct *queries =
      (struct queryStruct *)malloc(sizeof(struct queryStruct) * localQ);
  generateQueries(queries, localQ, my_rank);

  MPI_Barrier(MPI_COMM_WORLD);

  // Write code here
  double tStart, tEnd, t1, localTotalTime, totalTime, localTreeCreationTime,
      treeCreationTime, localSearchingTime, searchingTime;
  long unsigned int localSum, globalSum;

  RTree<int, double, 2, double> tree;

  // Start the time
  MPI_Barrier(MPI_COMM_WORLD);
  tStart = MPI_Wtime();

  /******************************************
   * Building tree for every data point
   *******************************************
   */

  for (int i = 0; i < localN; i++) {
    Rect rectangle = Rect(data[i].x, data[i].y, data[i].x, data[i].y);
    tree.Insert(rectangle.min, rectangle.max, i);
  }

  // End tree creation time && Start searching time
  MPI_Barrier(MPI_COMM_WORLD);
  t1 = MPI_Wtime();

  /******************************************
   * Range Queries in each ranks
   *******************************************
   */

  for (int i = 0; i < localQ; i++) {
    Rect search_rect = Rect(queries[i].x_min, queries[i].y_min,
                            queries[i].x_max, queries[i].y_max);

    unsigned int nhits =
        tree.Search(search_rect.min, search_rect.max, MySearchCallback, NULL);

    numResults[i] = nhits;
  }

  // End the time
  MPI_Barrier(MPI_COMM_WORLD);
  tEnd = MPI_Wtime();

  /******************************************
   * Time Calculation
   *******************************************
   */

  // Calculate local total time
  localTreeCreationTime = t1 - tStart;

  // Calculate local total time
  localSearchingTime = tEnd - t1;

  // Calculate local total time
  localTotalTime = tEnd - tStart;

  // Send the local tree creation time data and reduce into max value in rank 0
  MPI_Reduce(&localTreeCreationTime, &treeCreationTime, 1, MPI_DOUBLE, MPI_MAX,
             0, MPI_COMM_WORLD);

  // Send the local searching time data and reduce into max value in rank 0
  MPI_Reduce(&localSearchingTime, &searchingTime, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  // Send the local total time data and reduce into max value in rank 0
  MPI_Reduce(&localTotalTime, &totalTime, 1, MPI_DOUBLE, MPI_MAX, 0,
             MPI_COMM_WORLD);

  // Display max time measurements from rank 0
  if (my_rank == 0) {
    printf("#######################################\n");
    printf("Tree creation time: %f\n", treeCreationTime);
    printf("Searching time: %f\n", searchingTime);
    printf("TOTAL time taken: %f", totalTime);
    printf("\n#######################################\n");
  }

  /******************************************
   * Global Sum Calculation
   *******************************************
   */

  localSum = 0;
  for (int i = 0; i < localQ; i++) {
    localSum += numResults[i];
  }

  // Send localsum from all ranks to 0 and reduce the sum into global sum
  MPI_Reduce(&localSum, &globalSum, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0,
             MPI_COMM_WORLD);

  // Print global sum from rank 0
  if (my_rank == 0) {
    printf("#######################################\n");
    printf("Global Sum: %lu", globalSum);
    printf("\n#######################################\n");
  }

  /******************************************
   * Free memory allocations
   *******************************************
   */

  free(numResults);
  free(data);
  free(queries);

  MPI_Finalize();
  return 0;
}

// generates data [0,MAXVAL)
void generateData(struct dataStruct *data, unsigned int localN) {
  // seed rng do not modify
  // Same input dataset for all ranks
  srand(SEED);
  for (int i = 0; i < localN; i++) {
    data[i].x = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;
    data[i].y = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;
  }
}

// generates queries
// x_min y_min are in [0,MAXVAL]
// x_max y_max are x_min+d1 y_min+d2
// distance (d1)= [0, QUERYRNG)
// distance (d2)= [0, QUERYRNG)

void generateQueries(struct queryStruct *data, unsigned int localQ,
                     int my_rank) {
  // seed rng do not modify
  // Different queries for each rank
  srand(SEED + my_rank);
  for (int i = 0; i < localQ; i++) {
    data[i].x_min = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;
    data[i].y_min = ((double)rand() / (double)(RAND_MAX)) * MAXVAL;

    double d1 = ((double)rand() / (double)(RAND_MAX)) * QUERYRNG;
    double d2 = ((double)rand() / (double)(RAND_MAX)) * QUERYRNG;
    data[i].x_max = data[i].x_min + d1;
    data[i].y_max = data[i].y_min + d2;
  }
}