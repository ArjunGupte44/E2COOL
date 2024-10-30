#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <omp.h>
#include <vector>
#include <immintrin.h>
#include <array>

constexpr double PI(3.141592653589793);
constexpr double SOLAR_MASS(4 * PI * PI);
constexpr double DAYS_PER_YEAR(365.24);

struct Body {
    double x[3], v[3], mass;
    constexpr Body(double x0, double x1, double x2, double v0, double v1, double v2, double Mass)
        : x{x0, x1, x2}, v{v0, v1, v2}, mass(Mass) {}
};

class NBodySystem {
    std::vector<Body> bodies;

    void offset_momentum() {
        double p[3] = {0, 0, 0};
        #pragma omp parallel for reduction(+:p[0], p[1], p[2])
        for (auto &body : bodies) {
            for (int i = 0; i < 3; ++i)
                p[i] += body.v[i] * body.mass;
        }
        for (int i = 0; i < 3; ++i)
            bodies[0].v[i] -= p[i] / SOLAR_MASS;
    }

public:
    NBodySystem() : bodies({
        Body(0, 0, 0, 0, 0, 0, SOLAR_MASS),
        Body(4.84143144246472090e+00, -1.16032004402742839e+00, -1.03622044471123109e-01,
             1.66007664274403694e-03 * DAYS_PER_YEAR, 7.69901118419740425e-03 * DAYS_PER_YEAR,
             -6.90460016972063023e-05 * DAYS_PER_YEAR, 9.54791938424326609e-04 * SOLAR_MASS),
        Body(8.34336671824457987e+00, 4.12479856412430479e+00, -4.03523417114321381e-01,
             -2.76742510726862411e-03 * DAYS_PER_YEAR, 4.99852801234917238e-03 * DAYS_PER_YEAR,
             2.30417297573763929e-05 * DAYS_PER_YEAR, 2.85885980666130812e-04 * SOLAR_MASS),
        Body(1.28943695621391310e+01, -1.51111514016986312e+01, -2.23307578892655734e-01,
             2.96460137564761618e-03 * DAYS_PER_YEAR, 2.37847173959480950e-03 * DAYS_PER_YEAR,
             -2.96589568540237556e-05 * DAYS_PER_YEAR, 4.36624404335156298e-05 * SOLAR_MASS),
        Body(1.53796971148509165e+01, -2.59193146099879641e+01, 1.79258772950371181e-01,
             2.68067772490389322e-03 * DAYS_PER_YEAR, 1.62824170038242295e-03 * DAYS_PER_YEAR,
             -9.51592254519715870e-05 * DAYS_PER_YEAR, 5.15138902046611451e-05 * SOLAR_MASS)
        }) {
        offset_momentum();
    }

    void advance(double dt) {
        size_t n = bodies.size();
        #pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < n; ++i) {
            double r[3];
            for (size_t j = i + 1; j < n; ++j) {
                for (int m = 0; m < 3; ++m)
                    r[m] = bodies[i].x[m] - bodies[j].x[m];

                double dsquared = r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
                double distance = std::sqrt(dsquared);
                double mag = dt / (dsquared * distance);
                
                double force[3];
                for (int m = 0; m < 3; ++m) {
                    force[m] = r[m] * mag;
                    #pragma omp atomic
                    bodies[i].v[m] -= force[m] * bodies[j].mass;
                    #pragma omp atomic
                    bodies[j].v[m] += force[m] * bodies[i].mass;
                }
            }
        }

        #pragma omp parallel for
        for (size_t i = 0; i < n; ++i) {
            for (int m = 0; m < 3; ++m)
                bodies[i].x[m] += dt * bodies[i].v[m];
        }
    }

    double energy() const {
        double e = 0.0;
        for (auto bi = bodies.begin(); bi != bodies.end(); ++bi) {
            e += bi->mass * (bi->v[0] * bi->v[0] + bi->v[1] * bi->v[1] + bi->v[2] * bi->v[2]) / 2.0;
            for (auto bj = bi + 1; bj != bodies.end(); ++bj) {
                double distance = 0;
                for (int k = 0; k < 3; ++k) {
                    double dx = bi->x[k] - bj->x[k];
                    distance += dx * dx;
                }
                e -= (bi->mass * bj->mass) / std::sqrt(distance);
            }
        }
        return e;
    }
};

int main(int argc, char **argv) {
    int n = atoi(argv[1]);
    NBodySystem system;
    std::printf("%.9f\n", system.energy());
    for (int i = 0; i < n; ++i)
        system.advance(0.01);
    std::printf("%.9f\n", system.energy());
    return 0;
}