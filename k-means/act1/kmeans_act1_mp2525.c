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

  // Initialize time variables
  double tStart, tEnd, t0, t1, t2;

  // Initialize local time variables
  double localDistanceCalculationTime = 0.0;
  double localCentroidUpdateTime = 0.0;
  double localTotalTime;

  // Initialize global time variables
  double globalDistanceCalculationTime, globalCentroidUpdateTime,
      globalTotalTime;

  // Start the total time
  tStart = MPI_Wtime();

  /**
   * **************************************************
   *  Assign data range to all ranks by rank 0
   * **************************************************
   */

  int startRange, endRange;
  int *startRanges = (int *)malloc(sizeof(int) * nprocs);
  int *endRanges = (int *)malloc(sizeof(int) * nprocs);
  if (my_rank == 0) {
    for (int i = 0; i < nprocs; i++) {
      startRanges[i] = N / nprocs * i;
      if (N % nprocs != 0 && i == nprocs - 1) {
        endRanges[i] = startRanges[i] + N / nprocs + N % nprocs;
      } else {
        endRanges[i] = startRanges[i] + N / nprocs;
      }
    }
  }
  MPI_Scatter(startRanges, 1, MPI_INT, &startRange, 1, MPI_INT, 0,
              MPI_COMM_WORLD);
  MPI_Scatter(endRanges, 1, MPI_INT, &endRange, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Allocate memory for centroids and assign with first K values
  double **centroids = (double **)malloc(sizeof(double *) * KMEANS);
  for (int i = 0; i < KMEANS; i++) {
    centroids[i] = (double *)malloc(sizeof(double) * DIM);
    centroids[i] = dataset[i];
  }

  // Define clusters and global cluster count
  int *clusters = (int *)calloc(sizeof(int), N);
  int *globalClusterCount = (int *)calloc(sizeof(int), KMEANS);

  /**
   * **************************************************
   * Iterating K-Mean algorithm by KMEANSITERS times
   * **************************************************
   */

  // Allocate memory for partial mean
  double **partialMean = (double **)malloc(sizeof(double *) * KMEANS);
  for (int i = 0; i < KMEANS; i++) {
    partialMean[i] = (double *)calloc(sizeof(double), DIM);
  }

  // Define local cluster count
  int *localClusterCount = (int *)calloc(sizeof(int), KMEANS);

  for (int z = 0; z < KMEANSITERS; z++) {
    // Initialize partial mean with 0 in each iteration
    // Initialize local cluster count with 0 in each iteration
    for (int i = 0; i < KMEANS; i++) {
      for (int j = 0; j < DIM; j++) {
        partialMean[i][j] = 0.0;
      }
      localClusterCount[i] = 0;
    }

    /**
     * **************************************************
     * Distance Calculation
     * **************************************************
     */

    // Start distance calculation time for Zth iteration
    MPI_Barrier(MPI_COMM_WORLD);
    t0 = MPI_Wtime();

    for (int i = startRange; i < endRange; i++) {

      double *distances = (double *)calloc(sizeof(double), KMEANS);
      for (int j = 0; j < KMEANS; j++) {
        double *centroid = centroids[j];
        double *point = dataset[i];
        distances[j] = computeDistance(centroid, point, DIM);
      }

      // Get the minimum distance and assign point to cluster
      // associated with the centroid
      double minDistance = distances[0];
      int clusterIndex = 0;
      for (int x = 0; x < KMEANS; x++) {
        if (distances[x] < minDistance) {
          minDistance = distances[x];
          clusterIndex = x;
        }
      }

      // Assign cluster ID to point
      clusters[i] = clusterIndex;
      localClusterCount[clusterIndex] += 1;

    }

    // end distance calculation time for Zth iteration
    // Also, start time for centroid update
    MPI_Barrier(MPI_COMM_WORLD);
    t1 = MPI_Wtime();

    // calculate local distance calculation time
    localDistanceCalculationTime += t1 - t0;

    /**
     * **************************************************
     * Updating centroids and synchronization between ranks
     * **************************************************
     */

    // Reduce local cluster count into sum to get global cluster count
    MPI_Allreduce(localClusterCount, globalClusterCount, KMEANS, MPI_INT,
                  MPI_SUM, MPI_COMM_WORLD);

    // Calculate the partial mean from points in a cluster
    for (int i = startRange; i < endRange; i++) {
      for (int j = 0; j < KMEANS; j++) {
        if (clusters[i] == j) {
          for (int k = 0; k < DIM; k++) {
            partialMean[j][k] += dataset[i][k] / globalClusterCount[j];
          }
        }
      }
    }

    // Reduce partial mean into sum to get centroids
    for (int x = 0; x < KMEANS; x++) {
      MPI_Allreduce(partialMean[x], centroids[x], DIM, MPI_DOUBLE, MPI_SUM,
                    MPI_COMM_WORLD);
    }

    // end centroid update time for Zth iteration
    MPI_Barrier(MPI_COMM_WORLD);
    t2 = MPI_Wtime();

    // calculate local centroid update time
    localCentroidUpdateTime += t2 - t1;
  }

  // Stop total time
  MPI_Barrier(MPI_COMM_WORLD);
  tEnd = MPI_Wtime();

  // calculate local total time
  localTotalTime = tEnd - tStart;

  // Send local distance Calculation time data and reduce into max value
  MPI_Allreduce(&localDistanceCalculationTime, &globalDistanceCalculationTime,
                1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

  // Send local centroid update time data and reduce into max value
  MPI_Allreduce(&localCentroidUpdateTime, &globalCentroidUpdateTime, 1,
                MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

  // Send local total time data and reduce into max value
  MPI_Allreduce(&localTotalTime, &globalTotalTime, 1, MPI_DOUBLE, MPI_MAX,
                MPI_COMM_WORLD);

  if (my_rank == 0) {
    /*
    printf("#######################################\n");
    for (int j = 0; j < KMEANS; j++) {
      printf("Cluster %d: %d \n", j, globalClusterCount[j]);
    }
    */

    printf("#######################################\n");
    printf("Distance calculation time: %f\n", globalDistanceCalculationTime);
    printf("Centroid update time: %f\n", globalCentroidUpdateTime);
    printf("TOTAL time taken: %f", globalTotalTime);
    printf("\n#######################################\n");
    
  }

  /**
   * **************************************************
   * Free allocated memories
   * **************************************************
   */

  // free dataset
  for (int i = 0; i < N; i++) {
    free(dataset[i]);
  }
  free(dataset);

  free(startRanges);
  free(endRanges);

  // Commented, because it produced some weird error
  /*
  for (int i = 0; i < KMEANS; i++) {
    free(centroids[i]);
  }
  free(centroids);
  */

  for (int i = 0; i < KMEANS; i++) {
    free(partialMean[i]);
  }
  free(partialMean);

  free(globalClusterCount);
  free(localClusterCount);
  free(clusters);

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
