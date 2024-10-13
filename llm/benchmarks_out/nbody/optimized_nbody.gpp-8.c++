```cpp
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <immintrin.h>
#include <array>

constexpr double PI = 3.141592653589793;
constexpr double SOLAR_MASS = 4 * PI * PI;
constexpr double DAYS_PER_YEAR = 365.24;

struct Body {
    double x[3], v[3], mass;
    Body(double x0, double x1, double x2, double v0, double v1, double v2, double Mass)
        : x{x0, x1, x2}, v{v0, v1, v2}, mass(Mass) {}
};

class NBodySystem {
    static std::array<Body, 5> bodies;

    void offsetMomentum() {
        for (size_t k = 0; k < 3; ++k) {
            double momentum = 0.0;
            for (auto &body : bodies) {
                momentum += body.v[k] * body.mass;
            }
            bodies[0].v[k] -= momentum / SOLAR_MASS;
        }
    }

public:
    NBodySystem() {
        offsetMomentum();
    }

    void advance(double dt) {
        constexpr unsigned int N = ((bodies.size() - 1) * bodies.size()) / 2;
        static std::array<std::array<double, 3>, N> r;
        static std::array<double, N> mag;

        unsigned int i = 0;

        for (auto bi = bodies.begin(); bi != bodies.end(); ++bi) {
            for (auto bj = bi + 1; bj != bodies.end(); ++bj, ++i) {
                for (size_t m = 0; m < 3; ++m) {
                    r[i][m] = bi->x[m] - bj->x[m];
                }
            }
        }

        for (i = 0; i < N; i += 2) {
            __m128d dx[3], dsquared, distance, dmag;

            for (size_t m = 0; m < 3; ++m) {
                dx[m] = _mm_load_pd(&r[i][m]);
            }

            dsquared = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2];
            distance = _mm_cvtps_pd(_mm_rsqrt_ps(_mm_cvtpd_ps(dsquared)));

            for (size_t m = 0; m < 2; ++m) {
                distance = distance * _mm_set1_pd(1.5) - ((_mm_set1_pd(0.5) * dsquared) * distance) * (distance * distance);
            }

            dmag = _mm_set1_pd(dt) / dsquared * distance;
            _mm_store_pd(&mag[i], dmag);
        }

        i = 0;
        for (auto bi = bodies.begin(); bi != bodies.end(); ++bi) {
            for (auto bj = bi + 1; bj != bodies.end(); ++bj, ++i) {
                for (size_t m = 0; m < 3; ++m) {
                    double force = r[i][m] * mag[i];
                    bi->v[m] -= force * bj->mass;
                    bj->v[m] += force * bi->mass;
                }
            }
        }

        for (auto &body : bodies) {
            for (size_t m = 0; m < 3; ++m) {
                body.x[m] += dt * body.v[m];
            }
        }
    }

    double energy() const {
        double e = 0.0;
        for (auto bi = bodies.cbegin(); bi != bodies.cend(); ++bi) {
            e += 0.5 * bi->mass * (bi->v[0] * bi->v[0] + bi->v[1] * bi->v[1] + bi->v[2] * bi->v[2]);
            for (auto bj = bi + 1; bj != bodies.end(); ++bj) {
                double dx = bi->x[0] - bj->x[0];
                double dy = bi->x[1] - bj->x[1];
                double dz = bi->x[2] - bj->x[2];
                double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                e -= (bi->mass * bj->mass) / distance;
            }
        }
        return e;
    }
};

std::array<Body, 5> NBodySystem::bodies{{
    Body(0., 0., 0., 0., 0., 0., SOLAR_MASS),
    Body(4.84143144246472090e+00,
         -1.16032004402742839e+00,
         -1.03622044471123109e-01,
         1.66007664274403694e-03 * DAYS_PER_YEAR,
         7.69901118419740425e-03 * DAYS_PER_YEAR,
         -6.90460016972063023e-05 * DAYS_PER_YEAR,
         9.54791938424326609e-04 * SOLAR_MASS),
    Body(8.34336671824457987e+00,
         4.12479856412430479e+00,
         -4.03523417114321381e-01,
         -2.76742510726862411e-03 * DAYS_PER_YEAR,
         4.99852801234917238e-03 * DAYS_PER_YEAR,
         2.30417297573763929e-05 * DAYS_PER_YEAR,
         2.85885980666130812e-04 * SOLAR_MASS),
    Body(1.28943695621391310e+01,
         -1.51111514016986312e+01,
         -2.23307578892655734e-01,
         2.96460137564761618e-03 * DAYS_PER_YEAR,
         2.37847173959480950e-03 * DAYS_PER_YEAR,
         -2.96589568540237556e-05 * DAYS_PER_YEAR,
         4.36624404335156298e-05 * SOLAR_MASS),
    Body(1.53796971148509165e+01,
         -2.59193146099879641e+01,
         1.79258772950371181e-01,
         2.68067772490389322e-03 * DAYS_PER_YEAR,
         1.62824170038242295e-03 * DAYS_PER_YEAR,
         -9.51592254519715870e-05 * DAYS_PER_YEAR,
         5.15138902046611451e-05 * SOLAR_MASS)
}};

int main(int argc, char **argv) {
    int n = std::atoi(argv[1]);
    NBodySystem system;
    std::printf("%.9f\n", system.energy());
    for (int i = 0; i < n; ++i) {
        system.advance(0.01);
    }
    std::printf("%.9f\n", system.energy());
    return 0;
}
```