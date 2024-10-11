#include <stdio.h>
#include <stdlib.h>

void multiplyMatrices(float** first, float** second, float** result, int r1, int c1, int r2, int c2)
{

  
  for (int i = 0; i < r1; ++i) {
    for(int j = 0; j < c2; ++j) {
      for(int k = 0; k < c1; ++k) {
        result[i][j] += first[i][k] * second[k][j];
      }
    }
  }
}

int main(int argc, char* argv[]) {
 
  int r1, c1, r2, c2;
  int numMult = atoi(argv[1]);
  int matDim = atoi(argv[2]);
  float** first = (float**)malloc(matDim * sizeof(float*));
  float** second = (float**)malloc(matDim * sizeof(float*));
  float** result = (float**)malloc(matDim * sizeof(float*));
  for(int i = 0; i < matDim; i++)
  {
    first[i] = (float*)malloc(matDim * sizeof(float));
    second[i] = (float*)malloc(matDim * sizeof(float));
    result[i] = (float*)malloc(matDim * sizeof(float));
  }
  
 // printf("Num trials: %d\n", numMult);
  for(int i = 0; i < matDim; i++) {
    for(int j = 0; j < matDim; j++) {
      first[i][j] = i * j * 0.5;
      second[i][j] = (i + j) * 0.5;
      result[i][j] = 0;
    }
  }
  //printf("Finsihed filling matrices\n");
  for(int i = 0; i < numMult; i++)
  {
    multiplyMatrices(first,second, result, matDim, matDim, matDim, matDim);
  }
  printf("Result[5][5]: %lf\n", result[5][5]);
}
