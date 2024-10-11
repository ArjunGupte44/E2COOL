#include <vector>
#include <array>
#include <cmath>

constexpr double PI = 3.141592653589793;
constexpr double SOLAR_MASS = 4 * PI * PI;
constexpr double DAYS_PER_YEAR = 365.24;

struct body {
    double x[3], v[3], mass;
    constexpr body(double x0, double x1, double x2, double v0, double v1, double v2, double Mass)
        : x{x0, x1, x2}, v{v0, v1, v2}, mass(Mass) {}
};

class N_Body_System {
    static std::array<body, 5> bodies;

    // Precompute factors to reduce redundant calculations
    std::vector<std::vector<double>> precomputed_r;

    void offset_momentum()
    {
        for (auto& body : bodies) {
            for (int k = 0; k < 3; ++k) {
                bodies[0].v[k] -= body.v[k] * body.mass / SOLAR_MASS;
            }
        }
    }

public:
    N_Body_System()
    {
        offset_momentum();
        precomputed_r.resize(bodies.size(), std::vector<double>(bodies.size()));
    }

    void advance(double dt)
    {
        std::vector<double> mag(bodies.size() * (bodies.size() - 1) / 2);

        unsigned int index = 0;
        for (auto bi = bodies.begin(); bi != bodies.end(); ++bi) {
            auto bj = bi;
            for (++bj; bj != bodies.end(); ++bj, ++index) {
                for (int m = 0; m < 3; ++m) {
                    precomputed_r[bi - bodies.begin()][bj - bodies.begin()] = 
                        bi->x[m] - bj->x[m];
                }

                double dist_squared = precomputed_r[bi - bodies.begin()][bj - bodies.begin()]
                        [0] * precomputed_r[bi - bodies.begin()][bj - bodies.begin()][0] 
                        + precomputed_r[bi - bodies.begin()][bj - bodies.begin()][1] * 
                        precomputed_r[bi - bodies.begin()][bj - bodies.begin()][1] 
                        + precomputed_r[bi - bodies.begin()][bj - bodies.begin()][2] * 
                        precomputed_r[bi - bodies.begin()][bj - bodies.begin()][2];

                double distance = 1 / sqrt(dist_squared);
                
                mag[index] = dt * distance / dist_squared;

                for (int m = 0; m < 3; ++m) {
                    double force = precomputed_r[bi - bodies.begin()][bj - bodies.begin()] * mag[index];
                    bi->v[m] -= force * bj->mass;
                    bj->v[m] += force * bi->mass;
                }
            }
        }

        for (auto& body : bodies) {
            for (int m = 0; m < 3; ++m) {
                body.x[m] += dt * body.v[m];
            }
        }
    }

    double energy() const {
        double e = 0.0;
        for (auto bi = bodies.cbegin(); bi != bodies.cend(); ++bi) {
            e += bi->mass * (bi->v[0] * bi->v[0] + bi->v[1] * bi->v[1] + bi->v[2] * bi->v[2]) / 2.0;
            auto bj = bi;
            for (++bj; bj != bodies.cend(); ++bj) {
                double distance = 0.0;
                for (int k = 0; k < 3; ++k) {
                    double dx = bi->x[k] - bj->x[k];
                    distance += dx * dx;
                }
                e -= (bi->mass * bj->mass) / sqrt(distance);
            }
        }
        return e;
    }
};

std::array<body, 5> N_Body_System::bodies{{
    /* sun */
    body(0., 0., 0., 0., 0., 0., SOLAR_MASS),
    /* jupiter */
    body(4.84143144246472090e+00, -1.16032004402742839e+00, -1.03622044471123109e-01,
         1.66007664274403694e-03 * DAYS_PER_YEAR, 7.69901118419740425e-03 * DAYS_PER_YEAR,
         -6.90460016972063023e-05 * DAYS_PER_YEAR, 9.54791938424326609e-04 * SOLAR_MASS),
    /* saturn */
    body(8.34336671824457987e+00, 4.12479856412430479e+00, -4.03523417114321381e-01,
         -2.76742510726862411e-03 * DAYS_PER_YEAR, 4.99852801234917238e-03 * DAYS_PER_YEAR,
         2.30417297573763929e-05 * DAYS_PER_YEAR, 2.85885980666130812e-04 * SOLAR_MASS),
    /* uranus */
    body(1.28943695621391310e+01, -1.51111514016986312e+01, -2.23307578892655734e-01,
         2.96460137564761618e-03 * DAYS_PER_YEAR, 2.37847173959480950e-03 * DAYS_PER_YEAR,
         -2.96589568540237556e-05 * DAYS_PER_YEAR, 4.36624404335156298e-05 * SOLAR_MASS),
    /* neptune */
    body(1.53796971148509165e+01, -2.59193146099879641e+01, 1.79258772950371181e-01,
         2.68067772490389322e-03 * DAYS_PER_YEAR, 1.62824170038242295e-03 * DAYS_PER_YEAR,
         -9.51592254519715870e-05 * DAYS_PER_YEAR, 5.15138902046611451e-05 * SOLAR_MASS)
}};

int main(int argc, char** argv) {
    int n = std::atoi(argv[1]);
    N_Body_System system;

    printf("%.9f\n", system.energy());
    for (int i = 0; i < n; ++i) {
        system.advance(0.01);
    }
    printf("%.9f\n", system.energy());

    return 0;
}