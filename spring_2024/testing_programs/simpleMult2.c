#include <stdio.h>
#include <stdlib.h>

void multiplyMatrices(float* first, float* second, float* result, int r1, int c1, int r2, int c2)
{
  for (int i = 0; i < r1; ++i) {
    for (int k = 0; k < c1; ++k) {
      float temp = first[i * c1 + k]; // loop-invariant computation
      for (int j = 0; j < c2; ++j) {
        result[i * c2 + j] += temp * second[k * c2 + j];
      }
    }
  }
}

int main(int argc, char* argv[]) {

  int numMult = atoi(argv[1]);
  int matDim = atoi(argv[2]);
  int r1 = matDim;
  int c1 = matDim;
  int r2 = matDim;
  int c2 = matDim;

  float* first = (float*)malloc(r1 * c1 * sizeof(float));
  float* second = (float*)malloc(r2 * c2 * sizeof(float));
  float* result = (float*)calloc(r1 * c2, sizeof(float)); // initialize with zeros

  for (int i = 0; i < r1 * c1; i++) {
    first[i] = (i / c1) * (i % c1) * 0.5;
  }

  for (int j = 0; j < r2 * c2; j++) {
    second[j] = ((j / c2) + (j % c2)) * 0.5;
  }

  for (int i = 0; i < numMult; i++) {
    multiplyMatrices(first, second, result, r1, c1, r2, c2);
  }

  printf("Result[5][5]: %lf\n", result[5 * c2 + 5]);

  free(first);
  free(second);
  free(result);

  return 0;
}