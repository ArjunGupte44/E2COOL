#include <immintrin.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <omp.h>

int A_i_j(int i, int j) {
    // Ensure no zero division
    int div_i = (i / 2) + 1;
    int div_j = (j / 2) + 1;
    return div_i * div_j;
}

inline double indexA(int i, int j) {
    int divisor = A_i_j(i, j);
    // Protect against division by zero
    return 1.0 / (divisor != 0 ? divisor : 1);
}

void EvalPart_AVX2(double *src, double *dst, int begin, int end, int length) {
    int i = begin;
    for(; i + 3 < end; i += 4) {
        __m256d sum1 = _mm256_set1_pd(0.0);
        for (int j = 0; j < length; j++) {
            __m256d srcVal = _mm256_set1_pd(src[j]);
            __m256d idx = _mm256_set_pd(
                indexA(i+3, j),
                indexA(i+2, j),
                indexA(i+1, j),
                indexA(i  , j));
            sum1 = _mm256_add_pd(sum1, _mm256_div_pd(srcVal, idx));
        }
        _mm256_storeu_pd(dst + i, sum1);
    }
    // Handle remaining iterations safely
    for(; i < end; i++) {
        double sum = 0.0;
        for (int j = 0; j < length; j++)
            sum += src[j] / indexA(i, j);
        dst[i] = sum;
    }
}

void EvalAtATimesU_AVX(double *src, double *dst, double *tmp, int begin, int end, int N) {
    EvalPart_AVX2(src, tmp, begin, end, N);
    EvalPart_AVX2(tmp, dst, begin, end, N);
}

void initialize_vector(double *v, int size, double value) {
#pragma omp parallel for
    for(int i = 0; i < size; i++) {
        v[i] = value;
    }
}

double spectral_game(int N) {
    double *u = new double[N];
    double *v = new double[N];
    double *tmp = new double[N];

    initialize_vector(u, N, 1.0);
    
    for(int iter = 0; iter < 10; iter++) {
        EvalAtATimesU_AVX(u, v, tmp, 0, N, N);
        EvalAtATimesU_AVX(v, u, tmp, 0, N, N);
    }

    double vBv = 0.0, vv = 0.0;
    
    for(int i = 0; i < N; i++) {
        vBv += u[i] * v[i];
        vv += v[i] * v[i];
    }
    
    delete[] u;
    delete[] v;
    delete[] tmp;
    
    return sqrt(vBv / vv);
}

int main(int argc, char *argv[]) {
    int N = ((argc >= 2) ? atoi(argv[1]) : 2000);
    printf("%.9f\n", spectral_game(N));
    return 0;
}