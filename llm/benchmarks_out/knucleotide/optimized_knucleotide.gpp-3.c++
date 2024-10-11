#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdio>
#include <string>
#include <cstring>
#include <algorithm>
#include <unordered_map>
#include <future>
#include <vector>

unsigned char tonum[256], tochar[4];

static void init() {
    tonum['A'] = 0;
    tonum['C'] = 1;
    tonum['T'] = 2;
    tonum['G'] = 3;
    tochar[0] = 'A';
    tochar[1] = 'C';
    tochar[2] = 'T';
    tochar[3] = 'G';
}

struct T {
    uint64_t data;
    unsigned char size;

    T(const std::string& s = std::string())
        : data(0), size(s.size()) {
        reset(s, 0, s.size());
    }

    void reset(const std::string& s, unsigned beg, unsigned end) {
        size = end - beg;
        data = 0;
        for (unsigned i = beg; i != end; ++i) {
            data <<= 2;
            data |= tonum[unsigned(s[i])];
        }
    }

    bool operator<(const T& in) const {
        return data < in.data;
    }

    bool operator==(const T& in) const {
        return data == in.data;
    }

    operator std::string() const {
        std::string tmp;
        uint64_t tmp1 = data;
        for (unsigned i = 0; i != size; ++i) {
            tmp += tochar[tmp1 & 3];
            tmp1 >>= 2;
        }
        std::reverse(tmp.begin(), tmp.end());
        return tmp;
    }

    struct hash {
        std::size_t operator()(const T& t) const { return t.data; }
    };
};

using HashTable = std::unordered_map<T, unsigned, T::hash>;

HashTable calculate(const std::string& input, unsigned size, unsigned beg = 0, unsigned incr = 1) {
    HashTable frequencies;
    T tmp;
    for (unsigned i = beg, i_end = input.size() + 1 - size; i < i_end; i += incr) {
        tmp.reset(input, i, i + size);
        ++frequencies[tmp];
    }
    return frequencies;
}

HashTable tcalculate(const std::string& input, unsigned size) {
    unsigned N = std::thread::hardware_concurrency();
    std::vector<std::future<HashTable>> futures(N);
    for (unsigned i = 0; i < N; ++i)
        futures[i] = std::async(std::launch::async, calculate, std::ref(input), size, i, N);

    auto frequencies = futures[0].get();

    for (unsigned i = 1; i < N; ++i) {
        auto local_frequencies = futures[i].get();
        for (auto& kv : local_frequencies)
            frequencies[kv.first] += kv.second;
    }
    return frequencies;
}

void write_frequencies(const std::string& input, unsigned size) {
    unsigned sum = input.size() + 1 - size;
    auto frequencies = tcalculate(input, size);
    std::map<unsigned, std::string, std::greater<unsigned>> freq;
    for (auto& i : frequencies) {
        freq.insert(std::make_pair(i.second, std::string(i.first)));
    }
    for (auto& i : freq)
        std::cout << i.second << ' ' << (sum ? double(100 * i.first) / sum : 0.0) << '\n';
    std::cout << '\n';
}

void write_count(const std::string& input, const std::string& string) {
    unsigned size = string.size();
    auto frequencies = tcalculate(input, size);

    std::cout << frequencies[T(string)] << '\t' << string << '\n';
}

int main() {
    init();
    std::string input;
    char buffer[256];
    while (fgets(buffer, 100, stdin) && memcmp(">THREE", buffer, 6) != 0);
    while (fgets(buffer, 100, stdin) && buffer[0] != '>') {
        if (buffer[0] != ';') {
            input.append(buffer, strlen(buffer) - 1);
        }
    }
    std::transform(input.begin(), input.end(), input.begin(), ::toupper);

    std::cout << std::setprecision(3) << std::setiosflags(std::ios::fixed);
    write_frequencies(input, 1);
    write_frequencies(input, 2);
    write_count(input, "GGT");
    write_count(input, "GGTA");
    write_count(input, "GGTATT");
    write_count(input, "GGTATTTTAATT");
    write_count(input, "GGTATTTTAATTTATAGT");
}