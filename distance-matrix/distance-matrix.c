#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



//Example compilation
//mpicc distance_matrix_starter.c -lm -o distance_matrix_starter

//Example execution
//mpirun -np 1 -hostfile myhostfile.txt ./distance_matrix_starter 10000 90 MSD_year_prediction_normalize_0_1_100k.txt



//function prototypes
int importDataset(char * fname, int DIM, int N, double ** dataset);


int main(int argc, char **argv) {

  int my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);


  //Process command-line arguments
  int N;
  int DIM;
  char inputFname[500];


  if (argc != 4) {
    fprintf(stderr,"Please provide the following on the command line: N (number of lines in the file), dimensionality (number of coordinates per point), dataset filename. Your input: %s\n",argv[0]);
    MPI_Finalize();
    exit(0);
  }

  sscanf(argv[1],"%d",&N);
  sscanf(argv[2],"%d",&DIM);
  strcpy(inputFname,argv[3]);
  
  //pointer to dataset
  double ** dataset;
  
  if (N<1 || DIM <1)
  {
    printf("\nN is invalid or DIM is invalid\n");
    MPI_Finalize();
    exit(0);
  }
  //All ranks import dataset
  else
  {
   
    if (my_rank==0)
    {
    printf("\nNumber of lines (N): %d, Dimensionality: %d, Filename: %s\n", N, DIM, inputFname);
    }

    //allocate memory for dataset
    dataset=(double**)malloc(sizeof(double*)*N);

    for (int i=0; i<N; i++)
    {
      dataset[i]=(double*)malloc(sizeof(double)*DIM);
    }

    int ret=importDataset(inputFname, DIM, N, dataset);

    if (ret==1)
    {
      MPI_Finalize();
      return 0;
    }

  }

  //Write code here

  int * rowRanges;
  int * localRowRanges;
  double ** distanceMatrix;
  double ** sequentialDistanceMatrix;

  // Allocate memory for rowRanges and assign values in rank 0
  rowRanges = (int*)malloc(sizeof(int)*N);
  if(my_rank == 0) {
    for (int i=0; i<N; i++)
    {
      rowRanges[i] = i;
    }
  }

  // Allocate memory for local row ranges
  localRowRanges = (int*)malloc(sizeof(int)*(N/nprocs));
  
  // allocate memory for distance matrix
  distanceMatrix=(double**)malloc(sizeof(double*)*(N/nprocs));
  for (int i=0; i<N/nprocs; i++) {
    distanceMatrix[i]=(double*)malloc(sizeof(double)*N);
  }
  
  // allocate memory for sequential distance matrix
  sequentialDistanceMatrix = (double**)malloc(sizeof(double*)*N);

  for (int i=0; i<N; i++) {
    sequentialDistanceMatrix[i]=(double*)malloc(sizeof(double)*N);
  }

  // Print the dataset
  if(my_rank == 0) {
    printf("\nThe dataset is: \n");
    for (int i = 0; i<N; i++){
        for(int j = 0; j < DIM; j++) {
          printf("%.1f ", dataset[i][j]);
        }
      printf("\n");
    }
  }

  if(my_rank == 0) {
    // Sequential distance matrix calculation
      for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
          double distance = 0;
          for(int k = 0; k < DIM; k++) {
            distance += (dataset[i][k] - dataset[j][k]) * (dataset[i][k] - dataset[j][k]);
          }
          sequentialDistanceMatrix[i][j] = sqrt(distance);
        }
      }

      // Print sequential distance matrix
      printf("\nSequential Distance Matrix: \n");
      for (int i = 0; i<N; i++){
        for(int j = 0; j < N; j++) {
          printf("%.6f ", sequentialDistanceMatrix[i][j]);
        }
        printf("\n");
      }

      double seqGlobalSum = 0;
      // Calculate sequential global sum
      for (int i = 0; i<N; i++){
        for(int j = 0; j < N; j++) {
          seqGlobalSum += sequentialDistanceMatrix[i][j];
        }
      }
      printf("Sequential Global Sum is %0.3f\n", seqGlobalSum);
  }


  if(my_rank == 0) {
    printf("\nScatter Row ranges information to all ranks! \n\n");
  }

  MPI_Scatter(rowRanges, N/nprocs, MPI_INT, localRowRanges, N/nprocs, MPI_INT, 0, MPI_COMM_WORLD);
  
  printf("Rank %d local row ranges:", my_rank);
  for (int i = 0; i<N/nprocs; i++){
    printf("%i ", localRowRanges[i]);
  }
  printf("\n");

  // Distance matrix calculation
  for(int i = 0; i < N/nprocs; i++) {
    for(int j = 0; j < N; j++) {
      double distance = 0;
      for(int k = 0; k < DIM; k++) {

        int localIndex = localRowRanges[i];

        distance += (dataset[localIndex][k] - dataset[j][k]) * (dataset[localIndex][k] - dataset[j][k]);
      }
      distanceMatrix[i][j] = sqrt(distance);
    }
  }

  printf("Distance Matrix Value of rank %d: \n", my_rank);
  for (int i = 0; i<N/nprocs; i++){
      for(int j = 0; j < N; j++) {
        printf("%.6f ", distanceMatrix[i][j]);
      }
    printf("\n");
  }

  double globalSum;
  double localSum = 0;

  for (int i = 0; i<N/nprocs; i++){
      for(int j = 0; j < N; j++) {
        localSum += distanceMatrix[i][j];
      }
  }

  MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if(my_rank == 0) {
    printf("\nGlobal Sum is: %0.3f\n", globalSum);
  }

  //free dataset
  for (int i=0; i<N; i++)
  {
    free(dataset[i]);
  }

  for (int i=0; i<N/nprocs; i++)
  {
    free(distanceMatrix[i]);
  }

  free(dataset);
  free(rowRanges);
  free(localRowRanges);
  free(distanceMatrix);
  

  MPI_Finalize();

  return 0;
}




int importDataset(char * fname, int DIM, int N, double ** dataset)
{

    FILE *fp = fopen(fname, "r");

    if (!fp) {
        printf("Unable to open file\n");
        return(1);
    }

    char buf[4096];
    int rowCnt = 0;
    int colCnt = 0;
    while (fgets(buf, 4096, fp) && rowCnt<N) {
        colCnt = 0;

        char *field = strtok(buf, ",");
        double tmp;
        sscanf(field,"%lf",&tmp);
        dataset[rowCnt][colCnt]=tmp;

        
        while (field) {
          colCnt++;
          field = strtok(NULL, ",");
          
          if (field!=NULL)
          {
          double tmp;
          sscanf(field,"%lf",&tmp);
          dataset[rowCnt][colCnt]=tmp;
          }   

        }
        rowCnt++;
    }

    fclose(fp);



}

