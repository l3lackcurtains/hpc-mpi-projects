#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>

int main() {
  struct timeval start,end;

  
  gettimeofday(&start,NULL);

  
  int RowSize = 4;
  int N = 12;
  int b = 3;


  int bx = b;
  int by = b;
  
  if(RowSize < bx) {
    bx = RowSize;
  }

  int sum = 0;
  for(int x = 0; x < RowSize; x+= bx){

    for(int y = 0; y < N; y+= by) {

      for(int i = x; i < x + bx && i < RowSize; i++) {

        for(int j = y; j < y + by && j < N; j++) {

          printf("[%d %d] ", i, j);
          sum++;

        }
        printf("\n");
      }
    }
  }

  printf("Cache Sum: %d\n", sum);

  
  gettimeofday(&end,NULL);

  double elapsed = ((end.tv_sec*1000000.0 + end.tv_usec) -
            (start.tv_sec*1000000.0 + start.tv_usec)) / 1000000.00;
  printf("Elapsed time (s) : %.6f \n",elapsed);

  /*
  gettimeofday(&start,NULL);
  sum = 0;
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      printf("[%d %d] ", i, j);
      sum++;
    }
    printf("\n");
  }

  printf("Sum 2: %d\n", sum);

  gettimeofday(&end,NULL);

  double elapsed2 = ((end.tv_sec*1000000.0 + end.tv_usec) -
            (start.tv_sec*1000000.0 + start.tv_usec)) / 1000000.00;
  printf("No cache Elapsed time (s) : %.6f \n",elapsed2);

  */

  
  return 0;
}