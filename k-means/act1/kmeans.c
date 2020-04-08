// kmeans_starter.c

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KMEANSITERS 10

// compile
// mpicc kmeans.c -lm -o kmeans

// run example with 2 means
// mpirun -np 4 -hostfile myhostfile.txt ./kmeans 5159737 2 2
// iono_57min_5.16Mpts_2D.txt

// function prototypes
int importDataset(char *fname, int DIM, int N, double **dataset);
double computeDistance(int queryId, int pointId, int DIM, double **dataset);

int main(int argc, char **argv) {
  int my_rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  // Process command-line arguments
  int N;
  int DIM;
  int KMEANS;
  char inputFname[500];

  if (argc != 5) {
    fprintf(
        stderr,
        "Please provide the following on the command line: N (number of lines "
        "in the file), dimensionality (number of coordinates per point/feature "
        "vector), K (number of means), dataset filename. Your input: %s\n",
        argv[0]);
    MPI_Finalize();
    exit(0);
  }

  sscanf(argv[1], "%d", &N);
  sscanf(argv[2], "%d", &DIM);
  sscanf(argv[3], "%d", &KMEANS);
  strcpy(inputFname, argv[4]);

  // pointer to entire dataset
  double **dataset;

  if (N < 1 || DIM < 1 || KMEANS < 1) {
    printf("\nOne of the following are invalid: N, DIM, K(MEANS)\n");
    MPI_Finalize();
    exit(0);
  }
  // All ranks import dataset
  else {
    if (my_rank == 0) {
      printf(
          "\nNumber of lines (N): %d, Dimensionality: %d, KMEANS: %d, "
          "Filename: %s\n",
          N, DIM, KMEANS, inputFname);
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

  MPI_Barrier(MPI_COMM_WORLD);

  // Write code here
  int localN = N / nprocs;

  if (N % nprocs != 0 && my_rank == nprocs - 1) {
    localN = N / nprocs + N % nprocs;
  } else {
    localN = N / nprocs;
  }

  /******************************************
   * Assign data range to all ranks by rank 0
   *******************************************
   */

  int startingRange;
  int *startingRanges = (int *)malloc(sizeof(int) * nprocs);
  if (my_rank == 0) {
    for (int i = 0; i < nprocs; i++) {
      startingRanges[i] = N / nprocs * i;
    }
  }
  MPI_Scatter(startingRanges, 1, MPI_INT, &startingRange, 1, MPI_INT, 0,
              MPI_COMM_WORLD);
  int endRange = startingRange + localN;

  /******************************************
   * Initialize centroids with first K points
   *******************************************
   */
  int *centroids = (int *)malloc(sizeof(int) * KMEANS);
  for (int i = 0; i < KMEANS; i++) {
    centroids[i] = i * 10;
  }

  int *clusters = (int *)malloc(sizeof(int) * localN);
  int *clusterCount = (int *)calloc(sizeof(int), KMEANS);

if(my_rank == nprocs - 1) {
    for (int i = startingRange; i < endRange; i++) {

    double* distances = (double*) malloc(sizeof(double) * KMEANS);
    for (int j = 0; j < KMEANS; j++) {
      distances[j] = computeDistance(centroids[j], i, DIM, dataset);
    }
    double minDistance = distances[0];
    int clusterIndex = 0;
    for (int j = 0; j < KMEANS; j++) {
      if(distances[j] < minDistance) {
        minDistance = distances[j];
        clusterIndex = j;
      }
    }

    clusters[i] = clusterIndex;
    clusterCount[clusterIndex] += 1;
  }

  for (int j = 0; j < KMEANS; j++) {
    printf(" %d: %d \n", j, clusterCount[j]);
  }
}


  // free dataset
  for (int i = 0; i < N; i++) {
    free(dataset[i]);
  }

  free(dataset);
  MPI_Finalize();

  return 0;
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

double computeDistance(int queryId, int pointId, int DIM, double **dataset) {
  double dist = 0.00;
  for (int x = 0; x < DIM; x++) {
    dist += (dataset[queryId][x] - dataset[pointId][x]) *
            (dataset[queryId][x] - dataset[pointId][x]);
  }

  return sqrt(dist);
}
