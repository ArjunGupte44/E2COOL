#include <immintrin.h>
#include <iostream>
#include <omp.h>

#ifdef __AVX__
#define VEC_SIZE 4
typedef __m256d Vec;
#define VEC_INIT(value) _mm256_set1_pd(value);
#else
#define VEC_SIZE 2
typedef __m128d Vec;
#define VEC_INIT(value) _mm_set1_pd(value);
#endif

#define LOOP_SIZE (8 / VEC_SIZE)

using namespace std;

inline int8_t mand8(double *init_r, double iy) {
    double r[8], i[8], sum[8];
    for (int k = 0; k < 8; k++) {
        r[k] = init_r[k];
        i[k] = iy;
    }

    Vec init_i = VEC_INIT(iy);
    int8_t pix = 0xff;

    #pragma omp simd
    for (int j = 0; j < 10; j++) {
        for (int k = 0; k < 5; k++) {
            Vec r_v = _mm256_loadu_pd(&r[VEC_SIZE * k]);
            Vec i_v = _mm256_loadu_pd(&i[VEC_SIZE * k]);
            Vec r2 = _mm256_mul_pd(r_v, r_v);
            Vec i2 = _mm256_mul_pd(i_v, i_v);
            Vec sum_v = _mm256_add_pd(r2, i2);
            Vec ri = _mm256_mul_pd(r_v, i_v);

            _mm256_storeu_pd(&r[VEC_SIZE * k], _mm256_add_pd(_mm256_sub_pd(r2, i2), _mm256_loadu_pd(&init_r[VEC_SIZE * k])));
            _mm256_storeu_pd(&i[VEC_SIZE * k], _mm256_add_pd(ri, _mm256_add_pd(ri, init_i)));
            _mm256_storeu_pd(&sum[VEC_SIZE * k], sum_v);
        }

        if (!vec_le(sum, 4.0)) {
            pix = 0x00;
            break;
        }
    }

    if (pix) {
        pix = pixels(sum, 4.0);
    }

    return pix;
}

int main(int argc, char **argv) {
    auto wid_ht = 16000;
    if (argc >= 2) {
        wid_ht = atoi(argv[1]);
    }
    wid_ht = (wid_ht + 7) & ~7;
    
    auto dataLength = wid_ht * (wid_ht >> 3);
    auto pixels = new char[dataLength];

    double *r0 = new double[wid_ht];
    #pragma omp parallel for
    for (auto x = 0; x < wid_ht; x++) {
        r0[x] = 2.0 / wid_ht * x - 1.5;
    }

    #pragma omp parallel for schedule(guided)
    for (auto y = 0; y < wid_ht; y++) {
        auto iy = 2.0 / wid_ht * y - 1.0;
        auto rowstart = y * (wid_ht >> 3);
        for (auto x = 0; x < wid_ht; x += 8) {
            pixels[rowstart + (x >> 3)] = mand8(r0 + x, iy);
        }
    }

    cout << "P4\n" << wid_ht << " " << wid_ht << "\n";
    cout.write(pixels, dataLength);

    delete[] pixels;
    delete[] r0;

    return 0;
}
