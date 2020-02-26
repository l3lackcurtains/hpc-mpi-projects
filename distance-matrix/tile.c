#include <stdio.h>

int main() {
  int Row = 4;
  int N = 12;
  int bx = 3;
  int by = 3;
  int iStart = 0;
  int jStart = 0;
  int iEnd = 0;
  int jEnd = 0;

  if(Row < bx) {
    bx = Row;
  }

  while(iStart < Row){
    iEnd += bx;
    while(jStart < N) {
      jEnd += by;
      for(int i = iStart; i < iEnd && i < N; i++) {
        for(int j = jStart; j < jEnd && j < N; j++) {
          printf("[%d %d] ", i, j);
        }
        printf("\n");
      }
      printf("===\n");
      jStart = jEnd;
    }
    printf("===\n");
    iStart = iEnd;
    jStart = 0;
    jEnd = 0;
  }

  /*

  int b = 3;
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

  */

  return 0;
}