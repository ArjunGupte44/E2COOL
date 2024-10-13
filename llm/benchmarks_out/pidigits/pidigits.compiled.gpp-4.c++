#include <gmpxx.h>
#include <cstdlib>
#include <iostream>

class LFT {
public:
    mpz_class q;
    mpz_class r;
    mpz_class t;
    unsigned k;

public:
    LFT() : q(1), r(0), t(1), k(0){};

    void next() {
        ++k;
        r = (2 * k + 1) * (2 * q + r);
        t = (2 * k + 1) * t;
        q = q * k;
    }

    unsigned extract(unsigned x) const {
        static mpz_class tmp0, tmp1;
        tmp0 = q * x + r;
        tmp1 = tmp0 / t;
        return tmp1.get_ui();
    }

    void produce(unsigned n) {
        q = 10 * q;
        r = 10 * (r - n * t);
    }
};

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);

    const std::size_t TOTAL_DIGITS = std::atol(argv[1]);

    LFT lft;
    std::size_t n_digits = 0;
    
    // Buffer for batch output
    std::string output_buffer;

    while (n_digits < TOTAL_DIGITS) {
        lft.next();
        if (lft.q > lft.r) continue;

        auto digit = lft.extract(3);
        if (digit == lft.extract(4)) {
            // Append digit to buffer
            output_buffer.push_back('0' + digit);
            lft.produce(digit);
            ++n_digits;

            // Once every 10 digits or at end, print buffer
            if (n_digits % 10 == 0 || n_digits == TOTAL_DIGITS) {
                std::cout << output_buffer;

                // Pad digits with extra spaces if TOTAL_DIGITS was not a multiple of 10
                for (std::size_t i = output_buffer.length(); i < 10; ++i) std::cout << ' ';

                std::cout << "\t:" << n_digits << '\n';
                output_buffer.clear();
            }
        }
    }
}