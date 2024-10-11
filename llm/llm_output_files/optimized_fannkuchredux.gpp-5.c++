#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>

using namespace std;

static int64_t fact[32];

// Pre-calculate factorials up to n
void initializeFact(int n) {
    fact[0] = 1;
    for (int i = 1; i <= n; ++i) {
        fact[i] = i * fact[i - 1];
    }
}

class Permutation {
public:
    Permutation(int n, int64_t start);
    void advance();
    int64_t countFlips() const;

private:
    vector<int> count;
    vector<int8_t> current;
};

Permutation::Permutation(int n, int64_t start) : count(n), current(n) {
    for (int i = n - 1; i >= 0; --i) {
        auto d = start / fact[i];
        start %= fact[i];
        count[i] = d;
    }
    // Initialize current
    iota(current.begin(), current.end(), 0);
    for (int i = n - 1; i >= 0; --i) {
        rotate(current.begin(), current.begin() + count[i], current.begin() + i + 1);
    }
}

void Permutation::advance() {
    for (int i = 1; ;++i) {
        auto first = current[0];
        copy(current.begin() + 1, current.begin() + i + 1, current.begin());
        current[i] = first;
        if (++count[i] <= i) break;
        count[i] = 0;
    }
}

inline int64_t Permutation::countFlips() const {
    auto flips = 0;
    auto first = current[0];
    if (first > 0) {
        flips = 1;
        int8_t temp[32]; // assuming n <= 31
        copy(current.begin(), current.end(), temp);
        for (; temp[first] > 0; ++flips) {
            int8_t newFirst = temp[first];
            temp[first] = first;
            for (int low = 1, high = first - 1; low < high; ++low, --high) {
                swap(temp[low], temp[high]);
            }
            first = newFirst;
        }
    }
    return flips;
}

int main(int argc, char **argv) {
    const int n = atoi(argv[1]);
    if (n > 31) {
        cerr << "n too large, maximum supported is 31 due to int8_t overflow risk." << endl;
        return 1;
    }
    initializeFact(n);
    const int64_t blockLength = fact[n] / omp_get_max_threads();
    int64_t maxFlips = 0, checksum = 0;
    #pragma omp parallel for reduction(max:maxFlips) reduction(+:checksum)
    for (int64_t blockStart = 0; blockStart < fact[n]; blockStart += blockLength) {
        Permutation permutation(n, blockStart);
        auto index = blockStart;
        while (index < blockStart + blockLength) {
            int64_t flips = permutation.countFlips();
            checksum += (index % 2 == 0) ? flips : -flips;
            maxFlips = max(maxFlips, flips);
            permutation.advance();
            ++index;
        }
    }
    cout << checksum << endl;
    cout << "Pfannkuchen(" << n << ") = " << maxFlips << endl;
    return 0;
}