#include <gmpxx.h>
#include <iostream>
#include <vector>

class LFT {
public:
    mpz_class q, r, t;
    unsigned k;

    LFT() : q(1), r(0), t(1), k(0) {}

    void next() {
        ++k;
        mpz_class nr = (2 * k + 1) * (q + q + r);
        r = nr;
        t *= (2 * k + 1);
        q *= k;
    }

    unsigned extract(unsigned x) const {
        static mpz_class tmp0;
        tmp0 = q * x + r;
        return (tmp0 / t).get_ui();
    }

    void produce(unsigned n) {
        mpz_class nr = r - n * t;
        q *= 10;
        r = 10 * nr;
    }
};

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    const std::size_t TOTAL_DIGITS = std::atol(argv[1]);
    LFT lft;

    std::vector<unsigned> results;
    results.reserve(TOTAL_DIGITS);

    std::size_t n_digits = 0;
    unsigned digit3, digit4;

    while (n_digits < TOTAL_DIGITS) {
        lft.next();
        digit3 = lft.extract(3);
        digit4 = lft.extract(4);

        if (digit3 == digit4) {
            results.push_back(digit3);
            lft.produce(digit3);
            ++n_digits;
        }
    }

    for (std::size_t i = 0; i < results.size(); ++i) {
        if (i > 0 && i % 10 == 0) std::cout << "\t:" << i << '\n';
        std::cout << results[i];
    }
    std::cout << std::string(10 - (results.size() % 10), ' ') << "\t:" << results.size() << '\n';
    return 0;
}