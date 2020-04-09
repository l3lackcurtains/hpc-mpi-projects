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
double computeDistance(double *centroid, double *point, int dim);

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
  double **centroids = (double **)calloc(sizeof(double *), KMEANS);
  for (int i = 0; i < KMEANS; i++) {
    centroids[i] = (double *)calloc(sizeof(double), DIM);
    centroids[i] = dataset[i];
  }

  int *clusters = (int *)calloc(sizeof(int), N);

  int *globalClusterCount = (int *)calloc(sizeof(int), KMEANS);

  for (int z = 0; z < KMEANSITERS; z++) {

    double **partialMean = (double **)calloc(sizeof(double *), KMEANS);
    for (int i = 0; i < KMEANS; i++) {
      partialMean[i] = (double *)calloc(sizeof(double), DIM);
    }
    
    int *localClusterCount = (int *)calloc(sizeof(int), KMEANS);

    for (int i = startingRange; i < endRange; i++) {
      double *distances = (double *)calloc(sizeof(double), KMEANS);
      for (int j = 0; j < KMEANS; j++) {
        double *centroid = centroids[j];
        double *point = dataset[i];
        distances[j] = computeDistance(centroid, point, DIM);
      }
      double minDistance = distances[0];
      int clusterIndex = 0;
      for (int x = 0; x < KMEANS; x++) {
        if (distances[x] < minDistance) {
          minDistance = distances[x];
          clusterIndex = x;
        }
      }

      clusters[i] = clusterIndex;
      localClusterCount[clusterIndex] += 1;
    }

    // Send total cluster count
    MPI_Allreduce(localClusterCount, globalClusterCount, KMEANS, MPI_INT,
                  MPI_SUM, MPI_COMM_WORLD);

    // Calculate new centroid
    for (int i = startingRange; i < endRange; i++) {
      for (int j = 0; j < KMEANS; j++) {
        if (clusters[i] == j) {
          for (int k = 0; k < DIM; k++) {
            partialMean[j][k] += dataset[i][k];
          }
        }
      }
    }

    for (int x = 0; x < KMEANS; x++) {
      for (int y = 0; y < DIM; y++) {
        partialMean[x][y] = partialMean[x][y] / globalClusterCount[x];
      }
    }

    for (int x = 0; x < KMEANS; x++) {
    MPI_Allreduce(partialMean[x], centroids[x], DIM, MPI_DOUBLE,
                  MPI_SUM, MPI_COMM_WORLD);
    }
  }

  if (my_rank == 0) {   
    for(int i = 0; i < N; i++) {
      printf("%d, ", clusters[i]);
      
      for(int j = 0; j < DIM; j++) {
        if(j == DIM-1) {
          printf("%f\n", dataset[i][j]);
        } else {
          printf("%f, ", dataset[i][j]);
        }
      }
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

double computeDistance(double *centroid, double *point, int dim) {
  double dist = 0.00;
  for (int x = 0; x < dim; x++) {
    dist += (centroid[x] - point[x]) * (centroid[x] - point[x]);
  }

  return sqrt(dist);
}
