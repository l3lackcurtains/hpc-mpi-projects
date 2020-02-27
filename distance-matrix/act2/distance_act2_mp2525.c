#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Example compilation
// mpicc distance_matrix_starter.c -lm -o distance_matrix_starter

// Example execution
// mpirun -np 1 -hostfile myhostfile.txt ./distance_matrix_starter 10000 90
// MSD_year_prediction_normalize_0_1_100k.txt

// function prototypes
int importDataset(char *fname, int N, double **dataset);


int main(int argc, char **argv) {
  int my_rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  // Process command-line arguments
  int N;
  int DIM;
  int b;
  char inputFname[500];

  if (argc != 5) {
    fprintf(stderr,
            "Please provide the following on the command line: N (number of "
            "lines in the file), dimensionality (number of coordinates per "
            "point), dataset filename. Your input: %s\n",
            argv[0]);
    MPI_Finalize();
    exit(0);
  }

  sscanf(argv[1], "%d", &N);
  sscanf(argv[2], "%d", &DIM);
  sscanf(argv[3], "%d", &b);
  strcpy(inputFname, argv[4]);

  // pointer to dataset
  double **dataset;

  if (N < 1 || DIM < 1) {
    printf("\nN is invalid or DIM is invalid\n");
    MPI_Finalize();
    exit(0);
  }
  // All ranks import dataset
  else {
    if (my_rank == 0) {
      printf("\nNumber of lines (N): %d, Dimensionality: %d, Filename: %s\n", N,
             DIM, inputFname);
    }

    // allocate memory for dataset
    dataset = (double **)malloc(sizeof(double *) * N);

    for (int i = 0; i < N; i++) {
      dataset[i] = (double *)malloc(sizeof(double) * DIM);
    }

    int ret = importDataset(inputFname, N, dataset);

    if (ret == 1) {
      MPI_Finalize();
      return 0;
    }
  }

  // Write code here

  double t1, t2;

  if (my_rank == 0) {
    t1 = MPI_Wtime();
  }

  int *rowRanges;
  int *localRowRanges;
  double **distanceMatrix;

  // Allocate memory for rowRanges and assign values in rank 0
  rowRanges = (int *)malloc(sizeof(int) * N);
  if (my_rank == 0) {
    for (int i = 0; i < N; i++) {
      rowRanges[i] = i;
    }
  }

  // Allocate memory for local row ranges
  localRowRanges = (int *)malloc(sizeof(int) * (N / nprocs));

  // Scatter information about row ranges to all the ranks
  MPI_Scatter(rowRanges, N / nprocs, MPI_INT, localRowRanges, N / nprocs,
            MPI_INT, 0, MPI_COMM_WORLD);

  // allocate memory for distance matrix
  distanceMatrix = (double **)malloc(sizeof(double *) * (N / nprocs));
  for (int i = 0; i < N / nprocs; i++) {
    distanceMatrix[i] = (double *)malloc(sizeof(double) * N);
  }
  
  // Distance matrix calculation
  int rowSize = N / nprocs;
  int bx = b;
  int by = b;

  // Resize Rowsize if it is less than bx
  if(rowSize < bx) {
    bx = rowSize;
  }

  // Loop with step size of bx
  for(int x = 0; x < rowSize; x+= bx){
    // Loop with step size of by
    for(int y = 0; y < N; y+= by) {
      // Loop through row of matrix
      for(int i = x; i < x + bx && i < rowSize; i++) {
        // loop through column of matrix
        for(int j = y; j < y + by && j < N; j++) {
          double distance = 0;
          // Loop through dimention of matrix for distance calculation
          for (int k = 0; k < DIM; k++) {
            int localIndex = localRowRanges[i];
            distance += (dataset[localIndex][k] - dataset[j][k]) *
                        (dataset[localIndex][k] - dataset[j][k]);
          }
          distanceMatrix[i][j] = sqrt(distance);
        }
      }
    }
  }

  // Declare and initialize global and local sum
  double globalSum;
  double localSum = 0;

  // calculate the local sum in all ranks
  for (int i = 0; i < N / nprocs; i++) {
    for (int j = 0; j < N; j++) {
      localSum += distanceMatrix[i][j];
    }
  }

  // Send localsum from all ranks to 0 and reduce the sum into global sum
  MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  // Print global sum by rank 0
  if (my_rank == 0) {
    printf("Global Sum is: %f\n", globalSum);
  }

  // free dataset
  for (int i = 0; i < N; i++) {
    free(dataset[i]);
  }
  free(dataset);

  for (int i = 0; i < N / nprocs; i++) {
    free(distanceMatrix[i]);
  }
  free(distanceMatrix);

  // Free global and local row ranges
  free(rowRanges);
  free(localRowRanges);
  
  // Calculate and print the time elapsed in rank 0
  if (my_rank == 0) {
    t2 = MPI_Wtime();
    printf("Parallel Distance Matrix calculation time: %f seconds \n", t2 - t1);
  }

  MPI_Finalize();

  return 0;
}

int importDataset(char *fname, int N, double **dataset) {
  FILE *fp = fopen(fname, "r");

  if (!fp) {
    printf("Unable to open file\n");
    return (1);
  }

  char buf[4096];
  int rowCnt = 0;
  int colCnt = 0;
  while (fgets(buf, 4096, fp) && rowCnt < N) {
    colCnt = 0;

    char *field = strtok(buf, ",");
    double tmp;
    sscanf(field, "%lf", &tmp);
    dataset[rowCnt][colCnt] = tmp;

    while (field) {
      colCnt++;
      field = strtok(NULL, ",");

      if (field != NULL) {
        double tmp;
        sscanf(field, "%lf", &tmp);
        dataset[rowCnt][colCnt] = tmp;
      }
    }
    rowCnt++;
  }

  fclose(fp);
}