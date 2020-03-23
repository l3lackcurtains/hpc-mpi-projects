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