#include <iostream>
#include <cmath>
#include <vector>
#include <immintrin.h> // For SIMD intrinsics

constexpr double PI = 3.141592653589793; 
constexpr double SOLAR_MASS = 4 * PI * PI;
constexpr double DAYS_PER_YEAR = 365.24;

struct Body {
    double x[3], v[3], mass;
    constexpr Body(double x0, double x1, double x2, double v0, double v1, double v2, double Mass) :
        x{x0, x1, x2}, v{v0, v1, v2}, mass(Mass) {}
};

class NBodySystem {
    std::vector<Body> bodies;

    void offsetMomentum() {
        for (size_t i = 1; i < bodies.size(); ++i) {
            for (unsigned int k = 0; k < 3; ++k) {
                bodies[0].v[k] -= bodies[i].v[k] * bodies[i].mass / SOLAR_MASS;
            }
        }
    }

    void computeForces(double dt) {
        size_t size = bodies.size();
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = i + 1; j < size; ++j) {
                double dx = bodies[i].x[0] - bodies[j].x[0];
                double dy = bodies[i].x[1] - bodies[j].x[1];
                double dz = bodies[i].x[2] - bodies[j].x[2];

                double dsquared = dx * dx + dy * dy + dz * dz;
                double distance = std::sqrt(dsquared);
                double mag = dt / (dsquared * distance);

                for (unsigned int k = 0; k < 3; ++k) {
                    double rk = k == 0 ? dx : (k == 1 ? dy : dz);
                    bodies[i].v[k] -= rk * mag * bodies[j].mass;
                    bodies[j].v[k] += rk * mag * bodies[i].mass;
                }
            }
        }

        for (auto &body : bodies) {
            for (unsigned int m = 0; m < 3; ++m) {
                body.x[m] += dt * body.v[m];
            }
        }
    }

public:
    NBodySystem(std::vector<Body> init_bodies) : bodies(std::move(init_bodies)) {
        offsetMomentum();
    }

    void advance(double dt) {
        computeForces(dt);
    }

    double energy() const {
        double e = 0.0;
        for (auto bi = bodies.cbegin(); bi != bodies.cend(); ++bi) {
            e += bi->mass * (bi->v[0] * bi->v[0] + bi->v[1] * bi->v[1] + bi->v[2] * bi->v[2]) / 2.0;
            auto bj = bi;
            for (++bj; bj != bodies.end(); ++bj) {
                double distance2 = 0;
                for (auto k = 0; k < 3; ++k) {
                    const double dx = bi->x[k] - bj->x[k];
                    distance2 += dx * dx;
                }
                e -= (bi->mass * bj->mass) / std::sqrt(distance2);
            }
        }
        return e;
    }
};

int main(int argc, char **argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 1000;
    std::vector<Body> init_bodies = {  
        Body(0., 0., 0., 0., 0., 0., SOLAR_MASS),
        Body(4.84143144246472090e+00, -1.16032004402742839e+00, -1.03622044471123109e-01,
             1.66007664274403694e-03 * DAYS_PER_YEAR, 7.69901118419740425e-03 * DAYS_PER_YEAR, -6.90460016972063023e-05 * DAYS_PER_YEAR,
             9.54791938424326609e-04 * SOLAR_MASS),
        Body(8.34336671824457987e+00, 4.12479856412430479e+00, -4.03523417114321381e-01,
             -2.76742510726862411e-03 * DAYS_PER_YEAR, 4.99852801234917238e-03 * DAYS_PER_YEAR, 2.30417297573763929e-05 * DAYS_PER_YEAR,
             2.85885980666130812e-04 * SOLAR_MASS),
        Body(1.28943695621391310e+01, -1.51111514016986312e+01, -2.23307578892655734e-01,
             2.96460137564761618e-03 * DAYS_PER_YEAR, 2.37847173959480950e-03 * DAYS_PER_YEAR, -2.96589568540237556e-05 * DAYS_PER_YEAR,
             4.36624404335156298e-05 * SOLAR_MASS),
        Body(1.53796971148509165e+01, -2.59193146099879641e+01, 1.79258772950371181e-01,
             2.68067772490389322e-03 * DAYS_PER_YEAR, 1.62824170038242295e-03 * DAYS_PER_YEAR, -9.51592254519715870e-05 * DAYS_PER_YEAR,
             5.15138902046611451e-05 * SOLAR_MASS)
    };

    NBodySystem system(std::move(init_bodies));
    
    std::cout.precision(9);
    std::cout << system.energy() << std::endl;
    for (int i = 0; i < n; ++i) {
        system.advance(0.01);
    }
    std::cout << system.energy() << std::endl;

    return 0;
}
