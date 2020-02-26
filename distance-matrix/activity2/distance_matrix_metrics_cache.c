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
int importDataset(char *fname, int DIM, int N, double **dataset);
void sequentialDistanceMatrixCalculationWithTile(double **dataset, int N, int DIM, int b);


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

    int ret = importDataset(inputFname, DIM, N, dataset);

    if (ret == 1) {
      MPI_Finalize();
      return 0;
    }
  }

  // Write code here

  double t3, t4, t5, t6;

  if (my_rank == 0) {
    t3 = MPI_Wtime();
    sequentialDistanceMatrixCalculationWithTile(dataset, N, DIM, b);
    t4 = MPI_Wtime();

    printf("## With Tile\nSequential Distance Matrix calculation time: %f seconds\n",
           t4 - t3);
  }

  if (my_rank == 0) {
    t5 = MPI_Wtime();
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

  // allocate memory for distance matrix
  distanceMatrix = (double **)malloc(sizeof(double *) * (N / nprocs));
  for (int i = 0; i < N / nprocs; i++) {
    distanceMatrix[i] = (double *)malloc(sizeof(double) * N);
  }

  MPI_Scatter(rowRanges, N / nprocs, MPI_INT, localRowRanges, N / nprocs,
              MPI_INT, 0, MPI_COMM_WORLD);

  int rowSize = N / nprocs;
  // Distance matrix calculation
  int bx = b, by = b;
  int iStart = 0;
  int jStart = 0;
  int iEnd = 0;
  int jEnd = 0;

  if(rowSize < b) {
    bx = rowSize;
  }

  while(iStart < rowSize){
    iEnd += bx;
    while(jStart < N) {
      jEnd += by;
      for(int i = iStart; i < iEnd && i < N; i++) {
        for(int j = jStart; j < jEnd && j < N; j++) {
          double distance = 0;
          for (int k = 0; k < DIM; k++) {
            int localIndex = localRowRanges[i];

            distance += (dataset[localIndex][k] - dataset[j][k]) *
                        (dataset[localIndex][k] - dataset[j][k]);
          }
          distanceMatrix[i][j] = sqrt(distance);
        }
      }
      jStart = jEnd;
    }
    iStart = iEnd;
    jStart = 0;
    jEnd = 0;
  }

  double globalSum;
  double localSum = 0;

  for (int i = 0; i < N / nprocs; i++) {
    for (int j = 0; j < N; j++) {
      localSum += distanceMatrix[i][j];
    }
  }

  MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (my_rank == 0) {
    printf("\nGlobal Sum is: %0.3f\n\n", globalSum);
  }

  // free dataset
  for (int i = 0; i < N; i++) {
    free(dataset[i]);
  }

  for (int i = 0; i < N / nprocs; i++) {
    free(distanceMatrix[i]);
  }

  free(dataset);
  free(rowRanges);
  free(localRowRanges);
  free(distanceMatrix);

  if (my_rank == 0) {

    t6 = MPI_Wtime();
    
    printf("Parallel Distance Matrix calculation time: %f seconds\n", t6 - t5);

    printf("Parallel Speed Up: %f\n", (t4 - t3) / (t6 - t5));

    printf("Parallel Effeciency: %f\n", (t4 - t3) / (nprocs * (t6 - t5)));
    
  }

  MPI_Finalize();

  return 0;
}

void sequentialDistanceMatrixCalculationWithTile(double **dataset, int N, int DIM, int b) {
  double **sequentialDistanceMatrix;
  // allocate memory for sequential distance matrix
  sequentialDistanceMatrix = (double **)malloc(sizeof(double *) * N);
  for (int i = 0; i < N; i++) {
    sequentialDistanceMatrix[i] = (double *)malloc(sizeof(double) * N);
  }
  // Sequential distance matrix calculation

  // Distance matrix calculation
  int iStart = 0;
  int jStart = 0;
  int iEnd = 0;
  int jEnd = 0;

  while(iStart < N){
    iEnd += b;
    while(jStart < N) {
      jEnd += b;
      for(int i = iStart; i < iEnd && i < N; i++) {
        for(int j = jStart; j < jEnd && j < N; j++) {
          double distance = 0;
          for (int k = 0; k < DIM; k++) {
        distance +=
            (dataset[i][k] - dataset[j][k]) * (dataset[i][k] - dataset[j][k]);
      }
      sequentialDistanceMatrix[i][j] = sqrt(distance);
        }
      }
      jStart = jEnd;
    }
    iStart = iEnd;
    jStart = 0;
    jEnd = 0;
  }

  double seqGlobalSum = 0;
  // Calculate sequential global sum
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      seqGlobalSum += sequentialDistanceMatrix[i][j];
    }
  }
  printf("\nSequential Global Sum is %0.3f\n", seqGlobalSum);

  for (int i = 0; i < N; i++) {
    free(sequentialDistanceMatrix[i]);
  }
  free(sequentialDistanceMatrix);
}

int importDataset(char *fname, int DIM, int N, double **dataset) {
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
