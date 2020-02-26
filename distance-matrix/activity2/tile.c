#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>

int main() {
  struct timeval start,end;


  gettimeofday(&start,NULL);

  int nprocs = 4;
  int N = 11;

  int b = 3;
  int bx = 3;
  int iStart = 0;
  int jStart = 0;
  int iEnd = 0;
  int jEnd = 0;
  
  if(nprocs < bx) {
    bx = nprocs;
  }
  int sum = 0;
  while(iStart < nprocs){
    iEnd += bx;
    while(jStart < N) {
      jEnd += b;
      for(int i = iStart; i < iEnd && i < nprocs; i++) {
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

  printf("Cache Sum: %d\n", sum);

  
  gettimeofday(&end,NULL);

  double elapsed = ((end.tv_sec*1000000.0 + end.tv_usec) -
            (start.tv_sec*1000000.0 + start.tv_usec)) / 1000000.00;
  printf("Elapsed time (s) : %.6f \n",elapsed);


  gettimeofday(&start,NULL);
  sum = 0;
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      printf("[%d %d] ", i, j);
    }
    printf("\n");
  }

  gettimeofday(&end,NULL);

  double elapsed2 = ((end.tv_sec*1000000.0 + end.tv_usec) -
            (start.tv_sec*1000000.0 + start.tv_usec)) / 1000000.00;
  printf("No cache Elapsed time (s) : %.6f \n",elapsed2);

  
  return 0;
}